# UappsSDK Linux

## 简介
力合微电子的linux 平台的Uapps协议栈和SDK。用于主节点的软件开发，例如网关、中控屏等。

## 配置
1. 修改lme_uart_open的串口参数；
2. 根据需要修改main.cpp中的testMap用例内容，主要修改rsl和code字段；
3. 适配lme_gateway_log中的日志输出方式，默认为printf；
4. 适配内存申请和释放函数：Lme_free、Lme_malloc。

## 编译
先进入到src目录，然后执行
make clean; make
生成可执行文件uapps.bin

## 程序执行
sudo ./uapps.bin

可以通过日志输出，看到执行情况。
* 数据发送日志<br>
[2023-09-14 11:20:04:573 E 0001]lme_tool.c:PrintHexBytesInline:244:[send data:]:43:1b1b1b1b1b0a1b0a420200004567ffad0d6b31403230302e3463356130303065386430352f5f636c6f7365

* 数据接收日志<br>
[2023-09-14 11:22:43:945 E 0001]lme_tool.c:PrintHexBytesInline:244:[UAPPS msg:]:6:624502004e3a




