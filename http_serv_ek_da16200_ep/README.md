# HTTP server and sleep by button (Wakeup2 pin or A6) Demo

This is an example application for HTTP server and buttons on freeRTOS.

H/W setup:
1. GPIO : GPIOA6 (WAKEUP2 PIN) are used as buttons on EVK, SW4(1-4,2-3/don't care) should be on

## Example usage

This assume you went through the getting started and loaded and ran a get_started or example application on the DA16xxx.

- download & unzip the example project and locate it to the same folder for easy test like
  .\work\DA16200_DA16600_SDK_FreeRTOS_v3.2.X.X\
  .\work\http_serv_ek_da16200_ep\
  It could be any other folder.
  
- In the e2studio, select the latest SDK directory that you want to use as workspace.
- From the workspace you have setup go to **File->Import**
- Select **General->Dialog SDK Project** and click Next.
- In the `Select root directory` input the path where this example project is located.
- Select this project to import into the workspace and click Next.
- Make sure that the toolchain version in the dialog is 10.3.1.20210824 and select DA16200 as the target device.
- Click Finish.

## Run example 

1. Build http_serv_ek_da16200_ep project.
2. Download built image (in http_serv_ek_da16200_ep\img) to DA16200 by DA16200_FreeRTOS_Download.ttl.
3. Monitor the logs from UART0.
4. Provisioning should be done to start the HTTP server.
5. After boot up the HTTP server start and provide the web page.
6. It will be got into the sleep 2 mode 
7. If you press the button again, it will wake up and start the HTTP server again
8. Go to #6

##limitation

None

## Compatibility

- SDK v3.2.8
- SDK v3.2.9
