#define LOG(...) do {                                     \
        printf(__VA_ARGS__);                              \
        printf("\n");                                     \
    } while(0);
#define XDSERROR(...) do {                                   \
        printf(__VA_ARGS__);                              \
        printf("\n");                                     \
    } while(0);
