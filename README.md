# Container-Number-Manager

檢查由 monitor 得到的 server 及 application 資訊，與預先定好的規則(rules)做比較，判斷是否需要增加 application 使用的 pod/container 數量。
   
## Container_Number_Manager.cpp 
* 讀入 SLA.json 中 server 及 application 的 SLA 規則  
* 讀入 monitoringOutput.json 的觀測資訊，並判斷每個 application 是否需要增加 pod/container     

##### Compile & Run:   
```
make  
```

##local_lib
包含了json的lib跟include,在red hat的linux時不用安裝json。在非red hat的linux時不一定適用。

##JsonInput
Container_Number_Manager.cpp 所需要使用的json檔,包含：
 
#### SLA.json
server 及 application 的 SLA 規則
   
#### monitoringOutput.json  
monitor 得到的 server 及 application 資訊  

#### serverType.json
儲存 server 名稱及其對應類型的資訊
   
#### applicationInfo.json  
application的nodeSelector跟replication controller資訊,Container-Number-Manager在調整application的pod數量的instruction所需
  
