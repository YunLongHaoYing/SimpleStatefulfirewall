import request from '@/utils/request'

export function getConnetionList() {
  return request({
    url: '/myfw/connection/get',
    method: 'get'
  })
}

export function readPLog(data) {
  return request({
    url: '/myfw/log/readplog/' + data,
    method: 'get',
  })
}

export function readMLog(data) {
  return request({
    url: '/myfw/log/readmlog/' + data,
    method: 'get',
  })
}