# Container-Number-Manager

檢查由 monitor 得到的 server 及 application 資訊，與預先定好的規則(rules)做比較，判斷是否需要增加 application 使用的 pod/container 數量。
   
## Container_Number_Manager.cpp:  
* 讀入 SLA.json 中 server 及 application 的 SLA 規則  
* 讀入 example.json 的觀測資訊，並判斷每個 application 是否需要增加 pod/container     

##### Compile:   
```
make Container_Number_Manager  
```
##### Run:  
```
./Container_Number_Manager [-r rules] [-t server_type] [-m monitor file]  
```
* -r 預設為 SLA.json
* -t 預設為 serverType.json
* -m 預設為 example.json 
    
## SLA.json:  
server 及 application 的 SLA 規則

## serverType.json
儲存 server 名稱及其對應類型的資訊
   
## example.json  
monitor 輸出的觀測資訊，測試用

## format.json  
預設 monitor 輸出 / container number manager 輸入 格式
  
## CHT_configure_rules.py
互動式 SLA 規則修改 script
提供一個簡單的方式去更新 SLA.json 的內容
若同一資料夾中無 SLA.json，則新建立一個

##### Run:
```
python CHT_configure_rules.py
```