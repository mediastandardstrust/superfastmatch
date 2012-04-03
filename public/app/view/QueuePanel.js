Ext.define('Superfastmatch.view.QueuePanel', {
    extend: 'Ext.grid.Panel',
    alias: 'widget.queuepanel',
    requires: ['Superfastmatch.store.Queue'],
    itemId: 'QueuePanel',
    title: 'Queue',
    forceFit: true,
    columns: [
                {text:"ID",dataIndex:'id'},
                {text:"Status",dataIndex:'status'},
                {text:"Action",dataIndex:'action'},
                {text:"Priority",dataIndex:'priority'},
                {text:"Doc Type",dataIndex:'doctype'},
                {text:"Doc ID",dataIndex:'docid'},
                {text:"Source",dataIndex:'source'},
                {text:"Target",dataIndex:'target'}
              ],
    
    buildToolBar: function(store){
      return {
          xtype: 'pagingtoolbar',
          store: store,
          dock: 'bottom',
          displayInfo: true    
      }
    },
      
    initComponent: function() {
        var me = this,
            store=Ext.create('Superfastmatch.store.Queue');
        me.callParent(arguments);
        me.reconfigure(store);
        me.addDocked(me.buildToolBar(store));
    }
});
