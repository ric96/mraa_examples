#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include "mraa.h"

int running = 0;
static int iopin[] = {13,15,16};

void
sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("closing IO nicely\n");

        	running = -1;
	}
}

int main(int argc, char** arg)
{
	struct timespec t;
	mraa_result_t r = MRAA_SUCCESS;
	mraa_init();
	fprintf(stdout, "MRAA Version: %s\nStarting Blinking on IO\n", mraa_get_version());
	mraa_gpio_context gpio[(sizeof(iopin) / sizeof(iopin[0]))];
    
    //NESTED FUNCTIONS BEGIN

	int init_pin(int *i)
		{
			gpio[*i] = mraa_gpio_init(iopin[*i]);
			if(gpio[*i] == NULL)
			{
				fprintf(stderr, "Can't init pin %d \n", iopin[*i]);
				exit(1);
			}
			else
				fprintf(stdout, "init pin %d \n", iopin[*i]);
		}

	int direct_out(int *i)
		{
			r = mraa_gpio_dir(gpio[*i], MRAA_GPIO_OUT);
    		if (r != MRAA_SUCCESS)
			{
        		mraa_result_print(r);
    		}
			else
				fprintf(stdout, "set pin %d to out\n", iopin[*i]);
		}

	int direct_in(int *i)
		{
			r = mraa_gpio_dir(gpio[*i], MRAA_GPIO_IN);
    		if (r != MRAA_SUCCESS)
			{
        		mraa_result_print(r);
    		}
			else
				fprintf(stdout, "set pin %d to in\n", iopin[*i]);
		}

	int off(int *i)
		{
			r = mraa_gpio_write(gpio[*i], 0);
			if (r != MRAA_SUCCESS)
				{
   			     	mraa_result_print(r);
					return 1;
   			 	}
			else
				fprintf(stdout, "off pin %d \n", iopin[*i]);
		}
	int on(int *i)
		{
			r = mraa_gpio_write(gpio[*i], 1);
			if (r != MRAA_SUCCESS)
				{
   			     	mraa_result_print(r);
					return 1;
   			 	}
			else
				fprintf(stdout, "on pin %d \n", iopin[*i]);
		}

	int close_off(int *i)
		{
				//OFF
				off(&*i);
				direct_in(&*i);
				//CLOSE
				r = mraa_gpio_close(gpio[*i]);
   			 	if (r != MRAA_SUCCESS)
				{
   			     	mraa_result_print(r);
					return 1;
   			 	}
				else
					fprintf(stdout, "closed pin %d \n", iopin[*i]);
			//return 0;
		}
	//NESTED FUNCTIONS END

	//INIT PINS
	for(int i = 0; i < (sizeof(iopin) / sizeof(iopin[0])); i++)
	{
		init_pin(&i);
	}
	//SET DIRECTION TO OUT
	for(int i = 0; i < (sizeof(gpio) / sizeof(gpio[0])); i++)
	{
		direct_out(&i);
	}

	signal(SIGINT, sig_handler);

	while(running == 0)
	{
		for(int i = 0; i < (sizeof(gpio) / sizeof(gpio[0])); i++)
		{
			on(&i);
			nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
		}
		for(int i = (sizeof(gpio) / sizeof(gpio[0]))-1; i >= 0 ; i--)
		{
			off(&i);
			nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
		}
	}
	
	//OFF AND CLOSE ALL GPIOS
	for(int i = 0; i < (sizeof(gpio) / sizeof(gpio[0])); i++)
	{
		//OFF
		//CLOSE
		close_off(&i);
	}
    
	return r;
}

  
