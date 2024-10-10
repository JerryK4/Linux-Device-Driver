#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>

#define PWM_STATE _IOW('a','1',int32_t*)
#define GPIO_STATE _IOW('a','0',int32_t*)

void menu()
{
    printf("********************************\n");
    printf("**************MENU**************\n");
    printf("1:PWM_STATE\n");
    printf("2:GPIO_STATE\n");
    printf("0:Exit\n");
}

int main()
{
    int select;
    int32_t value;
    int fd=open("/dev/m_device",O_RDWR);
    if(fd < 0) {
            printf("Cannot open device file...\n");
            return 0;
    }
    while(1)
    {
        menu();
        printf("Your choice is : ");
        scanf("%d",&select);
        switch (select)
        {
        case 1:
            printf("PWM state\n");
            printf("Enter the value (0->100) to adjust led: ");
            scanf("%d",&value);
            ioctl(fd,PWM_STATE,(int32_t*)&value);
            break;
        case 2:
            printf("GPIO state\n");
            printf("Enter the value (0 or 1) to OFF/ON led: ");
            scanf("%d",&value);
            ioctl(fd,GPIO_STATE,(int32_t*)&value);
            break;
        case 0:
            printf("Exit\n");
            exit(1);
            break;
        }
    }
    close(fd);
}