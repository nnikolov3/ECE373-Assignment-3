/* Userspace program */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#define CHAR_DEVICE "/dev/pci_blink"

int32_t val;
int32_t user;
int32_t newval;
void toread();
int32_t ret, fd;
int32_t mask = 0x0000000F;
int32_t val;

int main ( void )
{
	/* File descriptor and returned value from write */

	printf (" This is for the user \n");
	/* Open the module */

	fd = open(CHAR_DEVICE, O_RDWR); /* Open the device*/

	if (fd < 0)
	{
		perror("Device is broken \n");
		exit(1);
	}

	printf("Reading from the device \n");
	int retval = read(fd,&ret,1);
	if (retval < 0)
	{
		perror ("Failed to read the message from the device. ");
		exit(1);
	}
	printf ("The current value is : %x in Hex  \n",ret);



	/* Write the new value for  */

	printf ("To turn on LED 0 , \n");
	printf("Type 0E \n");
	scanf("%x", &val);

	newval  = val;

	/* Write */
	ret = write (fd, &newval, 1);

	if (ret < 0)
	{
		perror("Failed to write !\n ");
		exit(1);
	}

	toread();

	return 0;

}

//The function to read the new value
void toread()
{
	do{
		printf ("Press 1 to read back 0 to exit\n");
		scanf("%x",&user);

	}while (user != 0 && user != 1);

	if (user == 0)
	{
		exit(0);

	}
	if (user == 1)
	{
		printf("Reading from the device \n");
		int r = read(fd,&ret, 1);
		if (r < 0)
		{
			perror ("Failed to read the message from the device. ");
			exit(1);
		}

		printf ("The value  is : %x in HEX \n",ret);

		sleep(2);

		printf(" Turn off LED by wrting 0F \n");
		scanf("%x",&val);

		newval = val;
		printf(" Turning off the led \n");

		/* Write */
		ret = write (fd, &newval,1);

		if (ret < 0)
		{
			perror("Failed to write !\n ");
			exit(1);
		}

		/* Read the new Value */

		printf("Reading from the device \n");
		 r = read(fd,&ret, 1);
		if (r < 0)
		{
			perror ("Failed to read the message from the device. ");
			exit(1);
		}
		printf ("The value  is : %x \n",ret);

		sleep(1);

		printf ("Goodbye \n");

		exit(0);

	}

}
