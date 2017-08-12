#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "mraa.h"

int running = 0;
static int outpin[] = {27,25,23};
static int inpin[] = {33,31,29};

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
	mraa_gpio_context gpio_out[(sizeof(outpin) / sizeof(outpin[0]))];
	mraa_gpio_context gpio_in[(sizeof(inpin) / sizeof(inpin[0]))];
    
    //NESTED FUNCTIONS BEGIN

	int init_pin(int *i, mraa_gpio_context *gpio, int *iopin)
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

	int direct_out(int *i, mraa_gpio_context *gpio, int *iopin)
		{
			r = mraa_gpio_dir(gpio[*i], MRAA_GPIO_OUT);
    		if (r != MRAA_SUCCESS)
			{
        		mraa_result_print(r);
    		}
			else
				fprintf(stdout, "set pin %d to out\n", iopin[*i]);
		}

	int direct_in(int *i, mraa_gpio_context *gpio, int *iopin)
		{
			r = mraa_gpio_dir(gpio[*i], MRAA_GPIO_IN);
    		if (r != MRAA_SUCCESS)
			{
        		mraa_result_print(r);
    		}
			else
				fprintf(stdout, "set pin %d to in\n", iopin[*i]);
		}

	int off(int *i, mraa_gpio_context *gpio, int *iopin)
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
	int on(int *i, mraa_gpio_context *gpio, int *iopin)
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
	int read_pin(int *i, mraa_gpio_context *gpio, int *iopin)
		{
			return mraa_gpio_read(gpio[*i]);
		}

	int close_off(int *i, mraa_gpio_context *gpio, int *iopin)
		{
				//OFF
				off(&*i, &*gpio, &*iopin);
				direct_in(&*i, &*gpio, &*iopin);
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
	int close(int *i, mraa_gpio_context *gpio, int *iopin)
		{
				direct_in(&*i, &*gpio, &*iopin);
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
	int led_thread()
		{
			//INIT PINS
			for(int i = 0; i < (sizeof(outpin) / sizeof(outpin[0])); i++)
			{
				init_pin(&i, gpio_out, outpin);
			}
			//SET DIRECTION TO OUT
			for(int i = 0; i < (sizeof(gpio_out) / sizeof(gpio_out[0])); i++)
			{
				direct_out(&i, gpio_out, outpin);
			}
		
			signal(SIGINT, sig_handler);
		
			while(running == 0)
			{
				for(int i = 0; i < (sizeof(gpio_out) / sizeof(gpio_out[0])); i++)
				{
					on(&i, gpio_out, outpin);
					nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
				}
				for(int i = (sizeof(gpio_out) / sizeof(gpio_out[0]))-1; i >= 0 ; i--)
				{
					off(&i, gpio_out, outpin);
					nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
				}
			}
			
			//OFF AND CLOSE ALL GPIOS
			for(int i = 0; i < (sizeof(gpio_out) / sizeof(gpio_out[0])); i++)
			{
				//OFF
				//CLOSE
				close_off(&i, gpio_out, outpin);
			}

		}

		int button_thread()
		{
			//INIT PINS
			for(int i = 0; i < (sizeof(inpin) / sizeof(inpin[0])); i++)
			{
				init_pin(&i, gpio_in, inpin);
			}
			//SET DIRECTION TO OUT
			for(int i = 0; i < (sizeof(gpio_in) / sizeof(gpio_in[0])); i++)
			{
				direct_in(&i, gpio_in, inpin);
			}
		    signal(SIGINT, sig_handler);
		
		    while (running == 0) {
				for(int i = 0; i < (sizeof(gpio_in) / sizeof(gpio_in[0])); i++)
				{
		        	fprintf(stdout, "Gpio %d is %d\n", inpin[i], read_pin(&i, gpio_in, inpin));
		        	nanosleep((const struct timespec[]){{0, 50000000L}}, NULL);
				}
		    }
		
			//OFF AND CLOSE ALL GPIOS
			for(int i = 0; i < (sizeof(gpio_in) / sizeof(gpio_in[0])); i++)
			{
				//CLOSE
				close(&i, gpio_in, inpin);
			}
		}
	//NESTED FUNCTIONS END

//	led_thread();
//	sleep(5);
//	running = 0;
//	button_thread();
	
	pthread_t led;
	pthread_t button;

	if(pthread_create(&button, NULL, button_thread, NULL)) 
	{

		fprintf(stderr, "Error creating thread\n");
		return 1;

	}

	if(pthread_create(&led, NULL, led_thread, NULL)) 
	{

		fprintf(stderr, "Error creating thread\n");
		return 1;

	}

	if(pthread_join(button, NULL)) 
	{

		fprintf(stderr, "Error joining thread\n");
		return 2;

	}

	if(pthread_join(led, NULL)) 
	{

		fprintf(stderr, "Error joining thread\n");
		return 2;

	}
	
	return r;
}

