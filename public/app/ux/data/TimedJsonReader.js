Ext.define('Ext.ux.data.TimedJsonReader', {
    extend: 'Ext.data.reader.Json',
    alias: 'reader.timedjson',

    getResponseData: function(response) {
        var data=this.callParent(arguments);
        data.responseTime=response.getResponseHeader('X-Response-Time');
        return data;
    }
});
