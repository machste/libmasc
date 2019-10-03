#include <masc.h>


static void ip_print_network_size(const Ip *self)
{
	uint8_t power = ip_max_prefixlen(self) - self->prefixlen;
	uint64_t hosts = (power <= 32) ? 1 << power : 0;
	print("hosts: ");
	if (power < 2) {
		print("%i", hosts);
	} else if (power <= 32) {
		print("%i (2^%i - 2)", hosts - 2, power);
	} else {
		print("2^%i - 2", power);
	}
	if (self->prefixlen == 0) {
		print(", biggest network for %s\n", ip_family_cstr(self));
	} else {
		print("\n");
	}
}

static void ip_print(const Ip *self)
{
	print("address: %O\n", self);
	if (ip_has_prefixlen(self)) {
		print("prefixlen: %i\n", self->prefixlen);
	}
	print("family: %s\n", ip_family_cstr(self));
	print("type: %s\n", ip_type_cstr(self));
	IpType type = ip_type(self);
	if (type != IP_TYPE_ADDR) {
		ip_print_network_size(self);
		if (type == IP_TYPE_CIDR_ADDR || type == IP_TYPE_BROADCAST) {
			Ip *network = ip_network_of(self);
			print("network: %O\n", network);
			delete(network);
		}
		if (type == IP_TYPE_CIDR_ADDR || type == IP_TYPE_NETWORK) {
			Ip *broadcast = ip_broadcast_of(self);
			print("broadcast: %O\n", broadcast);
			delete(broadcast);
		}
	}
}

int main(int argc, char *argv[])
{
	// Setup argument parser
	Argparse *ap = new(Argparse, path_basename(argv[0]), "IP Converter");
	argparse_add_opt(ap, 'v', "verbose", NULL, NULL, NULL, "invert regex match");
	argparse_add_arg(ap, "ip", "IP", NULL, argparse_ip, "IP address");
	// Parse command line arguments
	Map *args = argparse_parse(ap, argc, argv);
	Ip *ip = map_get(args, "ip");
	bool verbose = bool_get(map_get(args, "verbose"));
	// Print original IP
	if (verbose) ip_print(ip);
	// Convert IP to IPv6
	ip_convert_inet6(ip);
	if (verbose) {
		print("--- convert to IPv6 ---\n");
		ip_print(ip);
	} else {
		put(ip);
	}
	delete(ap);
	return 0;
}
