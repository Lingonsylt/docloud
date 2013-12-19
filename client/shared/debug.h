#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

/* Used for debugging */
#define log(str, ...) \
{ \
	FILE *fd; fd = fopen("c:\\temp\\log.txt", "a"); \
	fprintf(fd, str, __VA_ARGS__); \
	fclose(fd); \
	   }

#define logw(str, ...) \
{ \
	FILE *fd; fd = fopen("c:\\temp\\log.txt", "a"); \
	fwprintf(fd, str, __VA_ARGS__); \
	fclose(fd); \
}

#else
#define log(str, ...)
#define logw(str, ...)
#endif

#endif /* end of include guard: DEBUG_H */
