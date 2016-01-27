# Container-Number-Manager

## make_SLA_json.cpp:  
(名字待修改)  
將處理好的AP level imformation輸出至 SLA.json  
#### Compile:  
```
make make_SLA_json  
```
#### Run: 
```
./make_SLA_json.o  
```
  
## SLA.json:  
(名字待修改）  
儲存處理完的AP level imformation  

## Container_Number_Manager.cpp:  
* 讀入 SLA.json 的AP level imformation,儲存於map中  
* 讀入format.json中的machine imformation並儲存於structure中  
* 目前 Container_Number_Manager.cpp 的parser跟structure是針對 format.json 來寫,所以input為 example.json 或 format_original.json 時不會成功  
#### Compile:   
```
make Container_Number_Manager  
```
#### Run:  
```
./Container_Number_Manager.o format.json  
```

## format.json  
為目前使用的machine imformation輸入格式  

## example.json 跟 format_original.json  
為其他版本的machine imformation輸入格式  

  
