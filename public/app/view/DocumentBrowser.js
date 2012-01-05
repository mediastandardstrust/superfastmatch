Ext.define('Superfastmatch.view.DocumentBrowser', {
    extend: 'Ext.grid.GridPanel',
    alias: 'widget.documentbrowser',
    requires: ['Superfastmatch.model.Search','Superfastmatch.store.Documents'],

    itemId: 'DocumentBrowser',
    title: 'Saved Documents',
    forceFit: true,
    flex: 1,
    columns:[],
        
    initComponent: function() {
       var me=this,
           store=Ext.create('Superfastmatch.store.Documents');
       me.callParent(arguments);
       me.addDocked(Ext.create('Ext.toolbar.Paging',{
           itemId: 'DocumentPaging',
           displayInfo: true,
           dock: 'bottom'
       }));
       me.addEvents('documentselected','documentsloading');
       me.down('#DocumentPaging').bindStore(store,true);
       store.on('load',me.onLoad,me);
       me.on('select',me.onSelect,me);
       store.load();
    },
    
    onLoad: function(store,records,success){
        var me=this;
        me.reconfigure(store,store.model.getColumns());
        if (records.length){
            me.getSelectionModel().select(0);
        }
    },
    
    onSelect: function(selModel,selected){
        var me=this,
            search=Ext.ModelManager.getModel('Superfastmatch.model.Search'),
            id=selected.get('doctype')+'/'+selected.get('docid')+'/';
        me.fireEvent('documentsloading');
        search.load(id,{
            url: '/document/',
            success: me.onResults,
            scope: me
        });
    },
    
    onResults: function(record,operation){
        var me=this;
        me.fireEvent('documentselected',record);
    }
});