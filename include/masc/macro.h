#ifndef _MASC_MACRO_H_
#define _MASC_MACRO_H_

#define ARRAY_LEN(_array) (sizeof(_array) / sizeof(_array)[0])

#define LAMBDA(ret_type, fn_body) ({ret_type __fn__ fn_body; __fn__;})


#endif /* _MASC_MACRO_H_ */
