import request from '@/utils/request'

export function getRuleList() {
  return request({
    url: '/myfw/rule/get',
    method: 'get'
  })
}

export function addRule(data) {
    return request({
        url: '/myfw/rule/create',
        method: 'post',
        data
    })
}   

export function delRule(data) {
    return request({
        url: '/myfw/rule/delete',
        method: 'delete',
        data
    })
}

export function modifyRule(data) {
    return request({
        url: '/myfw/rule/modify',
        method: 'put',
        data
    })
}

export function getDefAct() {
    return request({
        url: '/myfw/setting/defact/get',
        method: 'get'
    })
}

export function setDefAct(data) {
    return request({
        url: '/myfw/setting/defact/set',
        method: 'post',
        data
    })
}