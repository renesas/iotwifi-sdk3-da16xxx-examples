## TLS server with multiple clients Demo

This is an example application for TLS server on freeRTOS.

## H/W setup:
1. DA16200 EVB required, it should be provisioned to the test AP
2. PC should be connected to the same AP

## Example usage

This assume you went through the getting started and loaded and ran a get_started or example application on the DA16xxx.

- download & unzip the example project and locate it to the same folder for easy test like
  .\work\DA16200_DA16600_SDK_FreeRTOS_v3.2.X.X\
  .\work\tls_serv_ek_da16200_ep\
  It could be any other folder.
  
- In the e2studio, select the latest SDK directory that you want to use as workspace.
- From the workspace you have setup go to **File->Import**
- Select **General->Dialog SDK Project** and click Next.
- In the `Select root directory` input the path where this example project is located.
- Select this project to import into the workspace and click Next.
- Make sure that the toolchain version in the dialog is 10.3.1.20210824 and select DA16200 as the target device.
- Click Finish.

## Run example 

1. Build tls_serv_ek_da16200_ep project.
2. Download built image (in tls_serv_ek_da16200_ep\img) to DA16200 by DA16200_FreeRTOS_Download.ttl.
3. Monitor the logs from UART0.
4. Provisioning should be done to start the test.
5. After boot up the TLS server start and wait for connection from TLS client.
6. In PC, run the command prompt in the location (.\tls_serv_ek_da16200_ep\tls_client1) and run the command like
   - tls_client_4k.exe [TLS server IP addr] [Port]
   - For example
     tls_client_4k.exe 192.168.0.4 10197
7. Open in another location/folder .\tls_serv_ek_da16200_ep\tls_client2, run the same command.

##limitation

No more clients will be connected if the heap memory is less than 30kB to do something internally for TCP.

## Compatibility

- SDK v3.2.9
