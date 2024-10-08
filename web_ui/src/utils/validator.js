/**
 * 是否合法IP地址
 * @param value
 * @param callback
 */
export function validateIP(rule, value, callback) {
    if (value === '' || typeof value === 'undefined' || value == null) {
      callback(new Error('请输入正确的IP地址'))
    } else {
      const reg = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/
      if ((!reg.test(value)) && value !== '') {
        callback(new Error('请输入正确的IP地址'))
      } else {
        callback()
      }
    }
}