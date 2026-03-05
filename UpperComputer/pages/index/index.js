// index.js
Page
({
  // 页面初始数据
  data:
  {
    time:"未知",
    user:"未知",
    air:"未知",
    lighting:"未知",
    paper:"未知",
    waterLevel:"未知",
    fan:"未知",
    waterPump:"未知",
    led:"未知",
    lock:"未知",
    door:"未知",
    method:"未知",
    reset:"未知",
    cmdPassDown:"未知",
    help:"未知"
  },

  // 配置 OneNET 平台参数
  config:
  {
    token:"version=2018-10-31&res=products%2Fr7vg0Nn75e%2Fdevices%2Fupper_computer&et=1827627080&method=md5&sign=6vOq3cBDqukXtL2m%2FHod%2Bg%3D%3D",
    productId:"r7vg0Nn75e",
    deviceName:"upper_computer",
    url_getInfo:'https://iot-api.heclouds.com/thingmodel/query-device-property?product_id=r7vg0Nn75e&device_name=upper_computer',
    url_setInfo:'https://iot-api.heclouds.com/thingmodel/set-device-property'
  },

  // 从 OneNET 获取设备状态
  OneNET_getInfo()
  {
    wx.request({

      url: this.config.url_getInfo,

      header:
      {
        'authorization':this.config.token
      },

      method:"GET",

      success:(e) =>
      {

        console.log(e)// 打印返回数据

        // 门状态映射（原始值 → 中文）
        const door_map = {
          "closeIn": "向内关",
          "closeOut": "向外关",
          "openIn": "向内开",
          "openOut": "向外开"
        };
        const temp_door = e.data.data[3].value;
        const display_door = door_map[temp_door] || temp_door || "未知";

        // 亮度映射
        const lighting_map = {
          "bright": "亮",
          "dark": "暗"
        };
        const temp_lighting = e.data.data[5].value;
        const display_lighting = lighting_map[temp_lighting] || temp_lighting || "未知";

        //设备管理方式映射
        const method_map = {
          "auto": "自动",
          "manual": "手动"
        };
        const temp_method = e.data.data[7].value;
        const display_method = method_map[temp_method] || temp_method || "未知";

         //纸巾剩余量映射
        const paper_map = {
          "many": "很多",
          "enough": "足够",
          "less": "较少",
          "needy": "极少"
        };
        const temp_paper = e.data.data[8].value;
        const display_paper = paper_map[temp_paper] || temp_paper || "未知";

        // 水位映射
        const waterLevel_map = {
          "normal": "正常",
          "high": "过高"
        };
        const temp_waterLevel = e.data.data[12].value;
        const display_waterLevel = waterLevel_map[temp_waterLevel] || waterLevel_map || "未知";

        // 气体浓度映射
        const air_map = {
          "normal": "正常",
          "high": "过高"
        };
        const temp_air = e.data.data[1].value;
        const display_air = air_map[temp_air] || temp_air || "未知";

        // 命令下传映射
        const cmdPassDown_map = {
          "cmdWindOn": "通风开",
          "cmdWindOff": "通风关",
          "cmdLedOn": "灯开",
          "cmdLedOff": "灯关",
          "cmdLockOn": "上锁",
          "cmdLockOff": "开锁",
          "cmdWaterPumpOn": "冲水开",
          "cmdWaterPumpOff": "冲水关",
          "cmdClIn": "命令内关",
          "cmdOpIn": "命令内开",
          "cmdClOut": "命令外关",
          "cmdOpOut": "命令外开",
          "cmdManual": "命令手动",
          "cmdAuto":  "命令自动",
          "cmdReset": "命令重置",
          "cmdFanFailed":  "通风失败",
          "cmdLedFailed":  "灯失败",
          "cmdLockFailed":  "锁失败",
          "cmdWaterPumpFailed":  "冲水失败",
          "cmdDoorFailed":  "门失败",
          "cmdMethodFailed":  "管理失败",
          "cmdResetFailed": "重置失败",
          "cmdUnknown": "未知命令",
          "cmdFailed": "未知命令",
          "noCmd" : "无命令"
        };
        const temp_cmdPassDown = e.data.data[2].value;
        const display_cmdPassDown = cmdPassDown_map[temp_cmdPassDown] || temp_cmdPassDown || "未知";

        // 更新页面数据
        this.setData
        ({

          led:e.data.data[0].value === "true" ? '开' : '关',
          air:display_air,
          cmdPassDown:display_cmdPassDown,
          door:display_door,
          help:e.data.data[4].value === "true" ? "求救！" : "暂无",
          lighting:display_lighting,
          lock:e.data.data[6].value === "true" ? "上锁" : "开锁",
          method:display_method,
          paper:display_paper,
          reset:e.data.data[9].value === "true" ? "重置中" : "已重置",
          time: e.data.data[10].value === "true" ? '是' : '否',
          user:e.data.data[11].value === "true" ? '是' : '否',
          waterLevel:display_waterLevel,
          waterPump:e.data.data[13].value === "true" ? '开' : '关',
          fan:e.data.data[14].value === "true" ? '开' : '关'
        })
      }
    })
  },

  // 页面加载时启动定时器，每2秒刷新一次数据
  onLoad()
  {
    setInterval(this.OneNET_getInfo, 2000)
  },

  // 通风开
  OneNET_setInfo_fan_on(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "wind" : true
          }
        }
      })
    }
  },

  // 通风关
  OneNET_setInfo_fan_off(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "wind" : false
          }
        }
      })
    }
  },

  // 灯开
  OneNET_setInfo_led_on(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "LED" : true
          }
        }
      })
    }
  },

  //灯关
  OneNET_setInfo_led_off(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "LED" : false
          }
        }
      })
    }
  },

  //上锁
  OneNET_setInfo_lock_on(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "lock" : true
          }
        }
      })
    }
  },

  //开锁
  OneNET_setInfo_lock_off(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "lock" : false
          }
        }
      })
    }
  },

  //水泵开
  OneNET_setInfo_waterPump_on(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "waterPump" : true
          }
        }
      })
    }
  },

  //水泵关
  OneNET_setInfo_waterPump_off(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "waterPump" : false
          }
        }
      })
    }
  },

  //门向内关
  OneNET_setInfo_door_closeIn(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "door" : "closeIn"
          }
        }
      })
    }
  },

  //门向内开
  OneNET_setInfo_door_openIn(event)
  {
    const is_checked = event.detail.value;

   
        wx.showToast  //按下按键后显示弹窗
      ({  
        title : '操作成功！',
        icon : 'success',
        duration : 1000  //弹窗自动隐藏时间
      })

      if(is_checked === true)
      {
      wx.request
      ({
        url: this.config.url_setInfo,

        header:
        {
          'authorization' : this.config.token
        },

        method : "POST",

        data:
        {
          "product_id" : this.config.productId,

          "device_name" : this.config.deviceName,

          "params" :
          {
            "door" : "openIn"
          }
        }
      })
    }
    
  },

  //门向外关
  OneNET_setInfo_door_closeOut(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "door" : "closeOut"
          }
        }
      })
    }
  },

  //门向外开
  OneNET_setInfo_door_openOut(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {
      wx.request
      ({
        url: this.config.url_setInfo,
  
        header:
        {
          'authorization' : this.config.token
        },
  
        method : "POST",
  
        data:
        {
          "product_id" : this.config.productId,
  
          "device_name" : this.config.deviceName,
  
          "params" :
          {
            "door" : "openOut"
          }
        }
      })
    }
  },
  
  //手动管理设备
  OneNET_setInfo_method_manual(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked === true)
    {
        wx.request
      ({
        url: this.config.url_setInfo,

        header:
        {
          'authorization' : this.config.token
        },

        method : "POST",

        data:
        {
          "product_id" : this.config.productId,

          "device_name" : this.config.deviceName,

          "params" :
          {
            "method" : "manual"
          }
        }
      })
    }
  },

  //自动管理设备
  OneNET_setInfo_method_auto(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked === true)
    {
        wx.request
      ({
        url: this.config.url_setInfo,

        header:
        {
          'authorization' : this.config.token
        },

        method : "POST",

        data:
        {
          "product_id" : this.config.productId,

          "device_name" : this.config.deviceName,

          "params" :
          {
            "method" : "auto"
          }
        }
      })
    }
  },

  //系统重置
  OneNET_setInfo_reset(event)
  {
    const is_checked = event.detail.value;

    wx.showToast  //按下按键后显示弹窗
    ({  
      title : '操作成功！',
      icon : 'success',
      duration : 1000  //弹窗自动隐藏时间
    })

    if(is_checked)
    {

      wx.request
      ({
        url: this.config.url_setInfo,

        header:
        {
          'authorization' : this.config.token
        },

        method : "POST",

        data:
        {
          "product_id" : this.config.productId,

          "device_name" : this.config.deviceName,

          "params" :
          {
            "reset" : is_checked
          }
        }
      })

    }
  },

})
