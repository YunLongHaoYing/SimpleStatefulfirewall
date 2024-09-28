import Axios from 'axios'
const axios = Axios.create({
  headers: {
    'Content-Type': 'application/json'
  },
  timeout: 5000, // 超时
  //baseURL: 'http://localhost:8888' // 请求接口地址，这里使用本项目地址，因为我们是前后端分离，后面需要在vue.config.js里面配置代理，实际请求得地址不是这个。
})

// 请求拦截
axios.interceptors.request.use(req => {
  return req
})

// 响应拦截
axios.interceptors.response.use(
  response => {
    const res = response.data
    console.log(res)

    // if the custom code is not 20000, it is judged as an error.
    if (res.err !== 0) {
      var msg = ""
      if (res.err === 40000)
        msg = "响应失败"
      else if (res.err === 42200)
        msg = "参数错误"
      return Promise.reject(new Error(msg || 'Error'))
    } else {
      return res
    }
  },
  error => {
    // 响应失败统一处理
    const { response } = error
    if (response) {
      switch (response.status) {
        case 400:
          //this..$message.error('请求无效')
          break
        case 401:
          //window.$vm.$message.error({ message: '尚未登录请重新登录' })
          break
        case 403:
          //window.$vm.$message.error('您没有权限这样做，请联系管理员')
          break
        case 404:
          //window.$vm.$message.error('请求未找到')
          break
        case 500:
          //window.$vm.$message.error('系统异常')
          break
        case 504:
          //window.$vm.$message.error('请求超时，请稍后再试')
          break
        default:
          //window.$vm.$message.error('系统异常')
          break
      }
    }
    return Promise.reject(error)
  }
)
export default axios
