#include <netdb.h>

#include <masc/net.h>
#include <masc/ip.h>


const char *net_af_to_cstr(int family)
{
	switch(family) {
	case AF_INET:
		return "inet";
	case AF_INET6:
		return "inet6";
	default:
		return "unknown";
	}
}

Str *net_gethostbyname(const char *hostname)
{
	Str *ip_str = NULL;

	struct hostent *host = gethostbyname(hostname);
	if (host != NULL) {
		Ip ip;
		ip_init_bin(&ip, host->h_addrtype, IP_NO_PREFIXLEN,
				host->h_addr_list[0]);
		ip_str = to_str(&ip);
		ip_destroy(&ip);
	}
	return ip_str;
}
