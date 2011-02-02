__attribute__ ((noreturn)) void _start(void);

void _exit(__attribute__((unused)) int n)
{
   while(1);
}

