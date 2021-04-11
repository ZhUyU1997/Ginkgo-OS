int main()
{
    asm volatile("ecall");
    while(1);
    return 0;
}