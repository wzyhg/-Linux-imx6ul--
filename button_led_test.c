//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
volatile int running = 1;
volatile int current_mode = 1;
/*
 * ./button_led_test /dev/lzp_button0 /dev/lzp_button1 /dev/lzp_led0 /dev/lzp_led1 /dev/lzp_led2 /dev/lzp_led3
 */
void set_led(int fd_led, int state) {
	char status = state;
	write(fd_led, &status, 1);
}

// 设置所有LED状态
void set_all_leds(int fd_led0, int fd_led1, int fd_led2, int fd_led3, int state) {
	set_led(fd_led0, state);
	set_led(fd_led1, state);
	set_led(fd_led2, state);
	set_led(fd_led3, state);
}
//模式1，所有LED亮
void flash_mode1(int fd_led0, int fd_led1, int fd_led2, int fd_led3) {
	set_all_leds(fd_led0, fd_led1, fd_led2, fd_led3, 0);
}
//模式2，所有LED亮
void flash_mode2(int fd_led0, int fd_led1, int fd_led2, int fd_led3, int* state) {
	if (*state == 0) {
		// 状态0: LED1和LED3亮，LED2和LED4灭
		set_led(fd_led0, 0);  // LED1亮
		set_led(fd_led1, 1);  // LED2灭
		set_led(fd_led2, 0);  // LED3亮
		set_led(fd_led3, 1);  // LED4灭
	
		printf("Mode 2: LED1&3 ● ON, LED2&4 ○ OFF\n");
		
	}
	else {
		// 状态1: LED1和LED3灭，LED2和LED4亮
		set_led(fd_led0, 1);  // LED1灭
		set_led(fd_led1, 0);  // LED2亮
		set_led(fd_led2, 1);  // LED3灭
		set_led(fd_led3, 0);  // LED4亮
		
		printf("Mode 2: LED1&3 ○ OFF, LED2&4 ● ON\n");
		
	}
	
	*state = !(*state);  // 切换状态
}

void flash_mode3(int fd_led0, int fd_led1, int fd_led2, int fd_led3, int* position) {
	// 先关闭所有LED
	set_all_leds(fd_led0, fd_led1, fd_led2, fd_led3, 1);

	// 点亮当前位置的LED
	switch (*position) {
	case 0:
		set_led(fd_led0, 0);  // LED1亮
		
		printf("Mode 3: ● LED1 ON, others OFF\n");
		break;
	case 1:
		set_led(fd_led1, 0);  // LED2亮
		
		printf("Mode 3: ● LED2 ON, others OFF\n");
		break;
	case 2:
		set_led(fd_led2, 0);  // LED3亮
		
		printf("Mode 3: ● LED3 ON, others OFF\n");
		break;
	case 3:
		set_led(fd_led3, 0);  // LED4亮
		
		printf("Mode 3: ● LED4 ON, others OFF\n");
		break;
	}

	// 移动到下一个位置
	*position = (*position + 1) % 4;
}
void flash_mode4(int fd_led0, int fd_led1, int fd_led2, int fd_led3) {
	set_all_leds(fd_led0, fd_led1, fd_led2, fd_led3, 1);
	printf("Mode 4: All LEDs OFF\n");
}




int main(int argc, char** argv)
{
	int fd_button0;
	int fd_button1;
	int fd_led0;
	int fd_led1;
	int fd_led2;
	int fd_led3;
	struct timespec last_mode_change, current_time;
	int mode2_state = 0;    // 模式2的状态
	int mode3_position = 0; // 模式3的位置

	char level0;
	char level1;

	int status;
	int status1;

	/* 1. 判断参数 */
	if (argc != 7) {
		return -1;
	}

	/* 2. 打开文件 */
	if ((fd_button0 = open(argv[1], O_RDWR)) == -1) {//打开button
		printf("can not open file %s\n", argv[1]);
		return -1;
	}
	if ((fd_button1 = open(argv[2], O_RDWR)) == -1) {
		printf("can not open  file %s\n", argv[2]);
		return -1;
	}
	if ((fd_led0 = open(argv[3], O_RDWR)) == -1) {//打开led
		printf("can not open file %s\n", argv[3]);
		return -1;
	}
	if ((fd_led1 = open(argv[4], O_RDWR)) == -1) {//打开led
		printf("can not open file %s\n", argv[4]);
		return -1;
	}
	if ((fd_led2 = open(argv[5], O_RDWR)) == -1) {//打开led
		printf("can not open file %s\n", argv[5]);
		return -1;
	}
	if ((fd_led3 = open(argv[6], O_RDWR)) == -1) {//打开led
		printf("can not open file %s\n", argv[6]);
		return -1;
	}

	clock_gettime(CLOCK_MONOTONIC, &last_mode_change);

	/* 3. 读文件 */
	while (1) {
		
		
		read(fd_button0, &level0, 1);
		read(fd_button1, &level1, 1);
		if (level0 == 0 && level1 == 0) {
			current_mode = 1;
			//flash_mode1(fd_led0, fd_led1, fd_led2, fd_led3);
		}
		else if (level0 == 1 && level1 == 0) {
			current_mode = 2;
		}
		else if (level0 == 1 && level1 == 1) {
			current_mode = 3;
		}
		else if (level0 == 0 && level1 == 1) {
			current_mode = 4;
		}

		
		switch (current_mode) {
		case 1: // 模式1：全亮
			flash_mode1(fd_led0, fd_led1, fd_led2, fd_led3);
			usleep(500000); // 0.5秒检查一次
			break;
		case 2: // 模式2：交替亮灭（1秒间隔）
			flash_mode2(fd_led0, fd_led1, fd_led2, fd_led3, &mode2_state);
			sleep(10);
			break;
		case 3: // 模式3：流水灯（0.5秒间隔）
			flash_mode3(fd_led0, fd_led1, fd_led2, fd_led3, &mode3_position);
			sleep(5); // 0.5秒
			break;
		case 4: // 模式4：全灭
			flash_mode4(fd_led0, fd_led1, fd_led2, fd_led3);
			usleep(500000); // 0.5秒检查一次
			break;


		}

	}

	close(fd_led0);
	close(fd_button0);
	close(fd_led1);
	close(fd_button1);
	close(fd_led2);
	close(fd_led3);
	return 0;
}




