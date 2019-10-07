#ifndef _MASC_NET_H_
#define _MASC_NET_H_

#include <masc/str.h>


const char *net_af_to_cstr(int family);

Str *net_gethostbyname(const char *hostname);

#endif /* _MASC_NET_H_ */
