# -Linux-imx6ul--
在Linux系统中，qemu环境下，使用imx6ull两个按键控制四个LED以不同模式频闪
参考视频，韦东山Linux驱动开发，参考文章：https://blog.csdn.net/qq_51103378/article/details/129663956?ops_request_misc=&request_id=&biz_id=102&utm_term=imx6ull%E5%A4%9A%E6%8C%89%E9%94%AE%E6%8E%A7%E5%88%B6&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduweb~default-3-129663956.142^v102^pc_search_result_base8&spm=1018.2226.3001.4187
Linux系统环境：韦东山 https://pan.baidu.com/s/1Gb80JoMsFxHvIUC3Gm-G7g,提取码 root
通过Xfpt将下载下来的代码上传到Linux系统中。
在Linux系统下，创建文件夹，将代码放进里面。cd指令进入代码文件夹下，执行make命令，得到button_drv.ko，chip_button_gpio.ko，led_drv.ko，chip_led_gpio.ko文件，执行arm-linux-gnueabihf-gcc -o button_led_test button_led_test.c，获得button_led_test文件
挂载nfs,qemu系统下访问文件夹，并执行insmod button_drv.ko;insmod led_drv.ko;insmod chip_led_gpio.ko;insmod chip_button_gpio.ko
qemu系统执行ls /dev |grep lzp，查看设备文件，应该包括lzp_button0，lzp_button1，lzp_led0,lzp_led1,lzp_led2,lzp_led3
最后在qemu系统下执行./button_led_test /dev/lzp_button0 /dev/lzp_button1 /dev/lzp_led0 /dev/lzp_led1 /dev/lzp_led2 /dev/lzp_led3，即可。
