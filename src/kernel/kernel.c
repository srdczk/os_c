// create a entry function 而不是 直接跳到 0x00
void dummy_test_entrypoint() {}

void main() {
    char *pos = (char *)(0xb8000);
    *pos = 'X';
}
