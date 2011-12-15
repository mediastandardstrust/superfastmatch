Ext.define('Ext.ux.data.FormWriter', {
    extend: 'Ext.data.writer.Writer',
    alternateClassName: 'Ext.data.FormWriter',
    alias: 'writer.form',

    // Only sends the first row of data
    // Should be fine as data comes from a form
    // TODO: consider multipart form
    writeRecords: function(request, data) {
        request.rawData=Ext.Object.toQueryString(data[0]);
        return request;
    }
});

