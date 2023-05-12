1.2023-04-24
1.1 把代码整合到一个目录中，jk板用到的所有软件，就都在这个目录中。


2.2023-04-27
2.1 mcu_update_uart_ymodem 已经基本稳定，升级功能正常。  ./xyzmodem_send -f hj22134-gd32f103-freertos.bin



3.2023-05-05
3.1 drv722api_reboot2 重启命令可用。


4. 2023-05-12
4.1 主要是从rk3399升级单片机的代码的增加
      mcu_check_version_hj_uart  用于查询单片机的版本，md5和编译时间
     mcu_update_uart_ymodem  用于给单片机从rk3399端升级。
