Ext.define('Superfastmatch.view.DocumentBrowser', {
    extend: 'Ext.grid.GridPanel',
    alias: 'widget.documentbrowser',
    requires: ['Superfastmatch.model.Search','Superfastmatch.store.Documents'],

    itemId: 'DocumentBrowser',
    title: 'Saved Documents',
    forceFit: true,
    flex: 1,
    columns: [{text:'Meta'}],
    viewConfig: {
        itemId: 'DocumentBrowserView',
        loadMask: true,
        emptyText: 'No Saved Documents Found'
    },
    
    buildToolBar: function(){
      var me=this;
      return Ext.create('Ext.toolbar.Toolbar',{
        itemId: 'DocumentPaging',
        dock: 'bottom',
        items:  [{
                  itemId: 'first',
                  tooltip: 'First Page',
                  iconCls: Ext.baseCSSPrefix + 'tbar-page-first',
                  handler: me.store.moveFirst,
                  scope: me.store
                },{
                  itemId: 'prev',
                  tooltip: 'Previous Page',
                  iconCls: Ext.baseCSSPrefix + 'tbar-page-prev',
                  handler: me.store.movePrevious,
                  scope: me.store
                },
                '-',
                'Doc Types:',
                Ext.create('Ext.form.field.Text',{
                     itemId: 'DocTypeFilter',
                     emptyText: 'eg. 1-3:6',
                     width:80,
                     regex:/^[\d:\-]*\d$/,
                     regexText: 'You can filter documents by ranges of Doc Type. For Example:.<br/>1-3 Show Doc Types 1,2 and 3<br/>1:4-5 Show Doc types 1, 4 and 5'
                }),
                '-',
                {
                  itemId: 'next',
                  tooltip: 'Next Page',
                  iconCls: Ext.baseCSSPrefix + 'tbar-page-next',
                  handler: me.store.moveNext,
                  scope: me.store
                },{
                  itemId: 'last',
                  tooltip: 'Last Page',
                  iconCls: Ext.baseCSSPrefix + 'tbar-page-last',
                  handler: me.store.moveLast,
                  scope: me.store
                },
                '->',
                'Documents:',
                {xtype: 'tbtext', itemId: 'documentCount'}
              ]
      });
    },
    
    initComponent: function() {
       var me=this;
       me.callParent(arguments);
       me.reconfigure(Ext.create('Superfastmatch.store.Documents'));
       me.addDocked(me.buildToolBar());
       me.addEvents('documentselected','documentsloading');
       me.store.on({
           beforeload: me.onBeforeLoad,
           load: me.onLoad,
           scope: me
       });
       me.on({
           select: me.onSelect,
           scope: me
       });
       me.down('#DocTypeFilter').on('change',me.onDoctypeFilterChange,me);
    },
    
    load: function(doc){
        var me=this;
        if (doc){
          me.getStore().sorters.clear();
          me.getStore().load({start: doc['doctype']+":"+doc['doctype']+":"+doc['docid']});
        }
        else if(me.getStore().count()==0){
          me.getStore().moveFirst();
        }
    },

    onDoctypeFilterChange: function(field){
      if(field.isValid()){
        this.getStore().moveFirst();
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
        me.down('#documentCount').setText(store.getTotalCount());
        me.getView().refresh();
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
        search.load(null,{
            url: '/document/'+id,
            success: me.onResults,
            scope: me
        });
    },
    
    onResults: function(record,operation){
        var me=this;
        me.fireEvent('documentselected',record);
    }
});