#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <masc/ip.h>
#include <masc/net.h>
#include <masc/cstr.h>
#include <masc/str.h>
#include <masc/num.h>
#include <masc/math.h>


static const char *type2cstr[] = {
	[IP_TYPE_UNKNOWN] = "unknown",
	[IP_TYPE_ADDR] = "address",
	[IP_TYPE_HOST] = "single-host",
	[IP_TYPE_P2P] = "point-to-point",
	[IP_TYPE_CIDR_ADDR] = "cidr-address",
	[IP_TYPE_NETWORK] = "network",
	[IP_TYPE_BROADCAST] = "boardcast"
};


Ip *ip_new(const char *ip)
{
	Ip *self = malloc(sizeof(Ip));
	ip_init(self, ip);
	return self;
}

void ip_init(Ip *self, const char *ip)
{
	object_init(self, IpCls);
	self->data = NULL;
	ip_set_cstr(self, ip);
}

static void _vinit(Ip *self, va_list va)
{
	char *ip = va_arg(va, char *);
	ip_init(self, ip);
}

Ip *ip_new_bin(uint8_t family, uint8_t prefixlen, void *data)
{
	Ip *self = malloc(sizeof(Ip));
	ip_init_bin(self, family, prefixlen, data);
	return self;
}

void ip_init_bin(Ip *self, uint8_t family, uint8_t prefixlen, void *data)
{
	object_init(self, IpCls);
	self->family = family;
	self->prefixlen = prefixlen;
	size_t size = ip_size(self);
	if (size > 0) {
		self->data = malloc(size);
		memcpy(self->data, data, size);
	}
}

Ip *ip_new_copy(const Ip *other)
{
	return ip_new_bin(other->family, other->prefixlen, other->data);
}

void ip_init_copy(Ip *self, const Ip *other)
{
	ip_init_bin(self, other->family, other->prefixlen, other->data);
}

void ip_destroy(Ip *self)
{
	free(self->data);
	self->family = AF_UNSPEC;
	self->data = NULL;
}

void ip_delete(Ip *self)
{
	ip_destroy(self);
	free(self);
}

bool ip_is_valid(Ip *self)
{
	return self->data != NULL;
}

size_t ip_size(const Ip *self)
{
	switch(self->family) {
	case AF_INET:
		return sizeof(struct in_addr);
	case AF_INET6:
		return sizeof(struct in6_addr);
	default:
		return 0;
	}
}

bool ip_has_prefixlen(const Ip *self)
{
	return self->prefixlen != IP_NO_PREFIXLEN;
}

uint8_t ip_max_prefixlen(const Ip *self)
{
	return ip_size(self) << 3;
}

const char *ip_family_cstr(const Ip *self)
{
	return net_af_to_cstr(self->family);
}

typedef bool (*_bytes_cb)(uint8_t *byte, void *args);
typedef bool (*_bits_cb)(uint8_t bit_mask, uint8_t *byte, void *args);

static bool _for_masked_data(Ip *self, _bytes_cb bytes_cb, _bits_cb bits_cb,
		void *args)
{
	uint8_t max_p_len = ip_max_prefixlen(self);
	int max_idx = ip_size(self) - 1;
	int last_byte = (max_p_len - self->prefixlen) / 8;
	int i;
	for (i = 0; i < last_byte; i++) {
		if (!bytes_cb(self->data + max_idx - i, args))
			return false;
	}
	int remaining_bits = (max_p_len - self->prefixlen) % 8;
	if (remaining_bits > 0) {
		uint8_t bit_mask = (1 << remaining_bits) - 1;
		if (!bits_cb(bit_mask, self->data + max_idx - i, args))
			return false;
	}
	return true;
}

static bool _check_bytes(uint8_t *orig_byte, void *args)
{
	uint8_t byte = *orig_byte;
	byte ^= *(bool *)args ? 0xff : 0x00;
	return byte == 0;
}

static bool _check_bits(uint8_t bit_mask, uint8_t *orig_byte, void *args)
{
	uint8_t byte = *orig_byte;
	byte ^= *(bool *)args ? 0xff : 0x00;
	return (byte & bit_mask) == 0;
}

IpType ip_type(const Ip *self)
{
	if (!ip_has_prefixlen(self)) {
		return IP_TYPE_ADDR;
	}
	uint8_t max_p_len = ip_max_prefixlen(self);
	if (self->prefixlen == max_p_len) {
		return IP_TYPE_HOST;
	} else if (self->prefixlen == max_p_len - 1) {
		return IP_TYPE_P2P;
	}
	bool inverted = false;
	if (_for_masked_data((Ip *)self, _check_bytes, _check_bits, &inverted))
		return IP_TYPE_NETWORK;
	inverted = !inverted;
	if (_for_masked_data((Ip *)self, _check_bytes, _check_bits, &inverted))
		return IP_TYPE_BROADCAST;
	return IP_TYPE_CIDR_ADDR;
}

const char *ip_type_cstr(const Ip *self)
{
	return type2cstr[ip_type(self)];
}

static bool _mask_bytes(uint8_t *byte, void *args)
{
	*byte  = *(bool *)args ? 0xFF : 0x00;
	return true;
}

static bool _mask_bits(uint8_t bit_mask, uint8_t *byte, void *args)
{
	if (*(bool *)args)
		*byte |= bit_mask;
	else
		*byte &= ~bit_mask;
	return true;
}

static Ip *_network_of(const Ip *self, bool inverted)
{
	Ip *ip = ip_new_copy(self);
	_for_masked_data(ip, _mask_bytes, _mask_bits, &inverted);
	return ip;
}

Ip *ip_network_of(const Ip *self)
{
	return _network_of(self, false);
}

Ip *ip_broadcast_of(const Ip *self)
{
	return _network_of(self, true);
}

bool ip_set_cstr(Ip *self, const char *ip_cstr)
{
	// Split IP and prefix parts
	Str net = init(Str, ip_cstr);
	List *ip_parts = str_split(&net, "/", 2);
	Str *ip = list_get_at(ip_parts, 0);
	// Free data pointer
	free(self->data);
	// Try to figure out whether it is an IPv4 or v6
	if (str_find(ip, ":") < 0) {
		// Expecting IPv4
		self->family = AF_INET;
	} else {
		// Expecting IPv6
		self->family = AF_INET6;
	}
	// Parse IP address
	self->data = malloc(ip_size(self));
	if (inet_pton(self->family, str_cstr(ip), self->data) != 1) {
		self->family = AF_UNSPEC;
		goto cleanup;
	}
	// Check for prefix
	if (len(ip_parts) == 2) {
		// Parse prefix
		char *prefix = str_cstr(list_get_at(ip_parts, 1));
		Int *prefixlen = cstr_to_int(prefix, true, NULL);
		if (prefixlen != NULL) {
			self->prefixlen = int_get(prefixlen);
			if (self->prefixlen > ip_max_prefixlen(self)) {
				self->family = AF_UNSPEC;
			}
		} else {
			self->family = AF_UNSPEC;
		}
		delete(prefixlen);
	} else {
		self->prefixlen = IP_NO_PREFIXLEN;
	}
cleanup:
	if (self->family == AF_UNSPEC) {
		self->prefixlen = IP_NO_PREFIXLEN;
		free(self->data);
		self->data = NULL;
	}
	delete(ip_parts);
	destroy(&net);
	return ip_is_valid(self);
}

ssize_t ip_to_n(const Ip *self, void *dst, size_t size)
{
	return -1;
}

bool ip_convert_inet6(Ip *self)
{
	if (self->family == AF_INET) {
		self->family = AF_INET6;
		self->data = realloc(self->data, sizeof(struct in6_addr));
		memcpy(self->data + 12, self->data, 4);
		memset(self->data, 0x00, 10);
		memset(self->data + 10, 0xff, 2);
		if (ip_has_prefixlen(self)) {
			self->prefixlen += 96;
		}
		return true;
	}
	return false;
}

int ip_cmp(const Ip *self, const Ip *other)
{
	return 0;
}

size_t ip_to_cstr(Ip *self, char *cstr, size_t size)
{
	long len = 0;
	if (self->family == AF_INET) {
		uint8_t *a = self->data;
		len = snprintf(cstr, size, "%u.%u.%u.%u", a[0], a[1], a[2], a[3]);
	} else if (self->family == AF_INET6) {
		char ip[INET6_ADDRSTRLEN];
		if (inet_ntop(self->family, self->data, ip, sizeof(ip)) != NULL) {
			len = cstr_ncopy(cstr, ip, size);
		}
	} else {
		return cstr_ncopy(cstr, "<invalid>", size);
	}
	// Append prefix length
	if (ip_has_prefixlen(self)) {
		len += snprintf(cstr + len, max(0, size - len), "/%u", self->prefixlen);
	}
	return len;
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _IpCls = {
	.name = "Ip",
	.size = sizeof(Ip),
	.super = NULL,
	.init_class = _init_class,
	.vinit = (vinit_cb)_vinit,
	.init_copy = (init_copy_cb)ip_init_copy,
	.destroy = (destroy_cb)ip_destroy,
	.cmp = (cmp_cb)ip_cmp,
	.repr = (repr_cb)ip_to_cstr,
	.to_cstr = (to_cstr_cb)ip_to_cstr,
};

const class *IpCls = &_IpCls;
