#ifndef _MASC_IP_H_
#define _MASC_IP_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <masc/object.h>


#define IP_NO_PREFIXLEN ( sizeof(((Ip *)0)->prefixlen) * 256 - 1 )


typedef enum {
	IP_TYPE_UNKNOWN,
	IP_TYPE_ADDR,
	IP_TYPE_HOST,
	IP_TYPE_P2P,
	IP_TYPE_CIDR_ADDR,
	IP_TYPE_NETWORK,
	IP_TYPE_BROADCAST
} IpType;

typedef struct {
	Object;
	uint8_t family;
	uint8_t prefixlen;
	uint8_t *data;
} Ip;


extern const class *IpCls;


Ip *ip_new(const char *ip);
void ip_init(Ip *self, const char *ip);

Ip *ip_new_bin(uint8_t family, uint8_t prefixlen, void *data);
void ip_init_bin(Ip *self, uint8_t family, uint8_t prefixlen, void *data);

void ip_destroy(Ip *self);
void ip_delete(Ip *self);

bool ip_is_valid(Ip *self);

size_t ip_size(const Ip *self);

bool ip_has_prefixlen(const Ip *self);

uint8_t ip_max_prefixlen(const Ip *self);

const char *ip_family_cstr(const Ip *self);

IpType ip_type(const Ip *self);
const char *ip_type_cstr(const Ip *self);

Ip *ip_network_of(const Ip *self);
Ip *ip_broadcast_of(const Ip *self);

bool ip_set_cstr(Ip *self, const char *ip_cstr);

ssize_t ip_to_n(const Ip *self, void *dst, size_t size);

bool ip_convert_inet6(Ip *self);

int ip_cmp(const Ip *self, const Ip *other);

size_t ip_to_cstr(Ip *self, char *cstr, size_t size);

#endif /* _MASC_IP_H_ */
