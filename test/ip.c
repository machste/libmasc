#include <masc/ip.h>
#include <masc/print.h>

int main(int argc, char *argv[])
{
	Ip *ip = new(Ip, "192.168.1.20/24");
	print("IPv4: %O\n", ip);
	ip_convert_inet6(ip);
	print("IPv6: %O\n", ip);
	delete(ip);
	return 0;
}