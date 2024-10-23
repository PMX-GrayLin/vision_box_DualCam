
![Red Badge](https://img.shields.io/badge/Important-Red-red)
# Project Title

![Important](https://img.shields.io/badge/Note-Important-red)

## MQTT command check list

### Main LED
$${\color{green}check status : PASS}$$
https://img.shields.io/badge/Status-PASS-green
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
$${\color{red}check status : Brightness not work (to check)}$$
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
$${\color{green}check status : PASS}$$
```
{
   "cmd":"IO_TOF_GET_PARAM",
   "args":{ }
}
```