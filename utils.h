#ifndef UTILS_H
#define UTILS_H


/** same as string.h
 * strchr but stops after
 * a certain count that will be
 * stop on the second occurance of ':' for ex
 * not the first as strchr(...) frok K&R behaves
 * @brief strchr2
 * @param str
 * @param c
 * @param count
 * @return
 */
const char *strchr2(const char* str, char c, int count);

/** my utility split function
 * @brief split
 * @param str - original string
 * @param delimiter - delimiter
 * @param ret_size - size of the splited elements to iter the array
 * @return 2d arry of chars spltted in separete row x col
 */
char** split(const char* str, char delimiter, int* ret_size);

#endif
