main(int ac, void **av) {
    int *buf;
    int i;
    int j;
    int k;
    int l;

    buf = &i;
    buf[0] = 0;
    buf[1] = 1;
    buf[2] = 2;
    buf[3] = 3;

    printf("%p\n", &i);
    printf("%p\n", &j);
    printf("%p\n", &k);
    printf("%p\n", &l);

    printf("saved base pointer: 0x%x%x\n", buf[5], buf[4]);
    printf("canary: 0x%x%x\n", buf[7], buf[6]);
    printf("return address: 0x%x%x\n", buf[9], buf[8]);

    buf[4] = 4;
    buf[5] = 5;
    buf[6] = 6;
    buf[7] = 7;
    buf[8] = 8;
    buf[9] = 9;
}
