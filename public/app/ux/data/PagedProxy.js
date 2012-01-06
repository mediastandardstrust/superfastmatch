Ext.define('Ext.ux.data.PagedProxy', {
    extend: 'Ext.data.proxy.Rest',
    alias: 'proxy.paged',
    requires: ['Ext.ux.data.FormWriter'],

    writer: 'form',
    noCache: false,
    pageParam: undefined,
    sortParam: 'order_by',
    startParam: 'cursor',
    
    encodeSorters: function(sorters) {
        return (sorters[0].direction=='DESC'?'-':'')+sorters[0].property;
    },
    
    buildRequest: function(operation){
        var me=this;
        if (operation.action=='create'){
            me.headers={'Content-Type':'application/x-www-form-urlencoded'};
        }else{
            me.headers={};
        }
        var request=me.callParent(arguments);
        return request;
    }
});

