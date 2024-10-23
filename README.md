
## MQTT command check list

### Main LED
![PASS](https://img.shields.io/badge/Status-PASS-green)
```
{
  "cmd":"IO_MAINLED_SET_PARAM",
  "args":{
  "lightSource":"1",
  "Brightness":3,
  "Switch":"ON"
  }
 }
```

 ### AI Lighting
![ToCheck](https://img.shields.io/badge/Status-ToCheck-red)
```
{
  "cmd":"IO_AILIGHTING_SET_PARAM",
  "args":{
  "lightSource":"[1,1,1,1]",
  "Brightness":1,
  "Switch":"ON"
  }
 }
```

 ### TOF
![PASS](https://img.shields.io/badge/Status-PASS-green)
```
{
   "cmd":"IO_TOF_GET_PARAM",
   "args":{ }
}
```