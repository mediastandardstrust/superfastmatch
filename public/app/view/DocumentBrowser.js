Ext.define('Superfastmatch.view.DocumentBrowser', {
    extend: 'Ext.grid.GridPanel',
    alias: 'widget.documentbrowser',
    requires: ['Superfastmatch.model.Search','Superfastmatch.store.Documents'],

    itemId: 'DocumentBrowser',
    title: 'Saved Documents',
    forceFit: true,
    flex: 1,
    columns:[],
    viewConfig: {
        itemId: 'DocumentBrowserView',
        emptyText: 'No Saved Documents Found'
    },
        
    initComponent: function() {
       var me=this,
           store=Ext.create('Superfastmatch.store.Documents');
       me.callParent(arguments);
       me.addDocked(Ext.create('Ext.toolbar.Paging',{
           itemId: 'DocumentPaging',
           displayInfo: true,
           items: ['-','Doc Types',Ext.create('Ext.form.field.Text',{
               itemId: 'DocTypeFilter',
               emptyText: 'eg. 1-3:6',
               width:150,
               regex:/^[\d:\-]*\d$/,
               regexText: 'You can filter documents by ranges of Doc Type. For Example:.<br/>1-3 Show Doc Types 1,2 and 3<br/>1:4-5 Show Doc types 1, 4 and 5'
               }),
               '-'
           ],    
           dock: 'bottom'
       }));
       me.addEvents('documentselected','documentsloading');
       me.down('#DocumentPaging').bindStore(store,true);
       store.on({
           beforeload: me.onBeforeLoad,
           load: me.onLoad,
           scope: me
       });
       me.on('select',me.onSelect,me);
       me.down('#DocTypeFilter').on('change',me.onDoctypeFilterChange,me);
    },
    
    onDoctypeFilterChange: function(field){
        if(field.isValid()){
          this.down('#DocumentPaging').doRefresh();  
        }
    },
    
    onBeforeLoad: function(store,operation,options){
        var me=this,
            doctypes=me.down('#DocTypeFilter');
        if (doctypes.isValid()){
            operation.doctypes=doctypes.getValue();     
        }
    },
    
    onLoad: function(store,records,success){
        var me=this;
        me.reconfigure(store,store.model.getColumns());
        if (records.length){
            me.getSelectionModel().select(0);
        }else{
            me.fireEvent('documentselected',null);
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