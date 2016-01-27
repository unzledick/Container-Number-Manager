# Container-Number-Manager
   
   
## make_SLA_json.cpp:  
將處理好的 AP level imformation 輸出至 SLA.json  
(名字待修改)  
##### Compile:  
```
make make_SLA_json  
```
##### Run: 
```
./make_SLA_json.o  
```
   
    
## SLA.json:  
儲存處理完的AP level imformation  
(名字待修改）  
   
   
## Container_Number_Manager.cpp:  
* 讀入 SLA.json 的 AP level imformation,儲存於 map 中 (執行會印出結果）  
* 讀入 format.json 中的 machine imformation 並儲存於 structure 中 (執行會印出結果）  
* 目前 Container_Number_Manager.cpp 的 parser 跟 structure 是針對 format.json 來寫,  
所以input為 example.json 或 format_original.json 時不會成功   

##### Compile:   
```
make Container_Number_Manager  
```
##### Run:  
```
./Container_Number_Manager.o format.json  
```
   
   
## format.json  
目前使用的 machine imformation 輸入格式  
   
   
## example.json & format_original.json  
其他版本的 machine imformation 輸入格式  
