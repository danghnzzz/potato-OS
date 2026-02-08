#ifndef STDARG_H
#define STDARG_H

#define _va_align(type) ((sizeof(type) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, last_arg) (ap = ((va_list) &last_arg) + _va_align(last_arg))
#define va_arg(ap, type) (*((type *) ((ap += _va_align(type)) - _va_align(type))))
#define va_end(ap) (ap = (va_list) 0)

typedef char *va_list;

#endif
