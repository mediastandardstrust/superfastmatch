Ext.define('Ext.ux.panel.HTMLPanel', {
    extend: 'Ext.panel.Panel',
    alias: 'widget.HTMLPanel',

    autoScroll: true,
    loader: {
        scripts: true,
        loadMask: true
    },

    initComponent: function() {
        var me = this;
        me.callParent(arguments);
        me.on('activate',function(tabpanel){tabpanel.loader.load();});
    }
});