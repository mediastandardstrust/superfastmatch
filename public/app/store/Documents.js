Ext.define('Superfastmatch.store.Documents', {
    extend: 'Ext.data.Store',
    requires: ['Superfastmatch.model.Document'],
    model: 'Superfastmatch.model.Document',
    storeId: 'Documents',
    pageSize: 10,
    remoteSort: true,

    priorPage: 1,
    cursors: {},
    
    constructor: function(){
        var me = this;
        me.callParent(arguments);
        me.on({
            load: me.onLoad,
            beforeload: me.beforeLoad,
            scope: me
        });
    },

    resetCursors: function(){
      this.cursors={};
    },

    beforeLoad: function(store,operation){
        var me=this,
            pageDifference=me.currentPage-me.priorPage;
        if (pageDifference==0){
            operation.start=me.cursors.current?me.cursors.current:'';
        }else if (pageDifference<-1){
            operation.start=me.cursors.first;
        }else if (pageDifference==-1){
            operation.start=me.cursors.previous;
        }else if(pageDifference==1){
            operation.start=me.cursors.next;
        }else if(pageDifference>1){
            operation.start=me.cursors.last;
        };
    },
    
    onLoad: function(store,records,successful){
        if (successful){
            store.cursors=store.getProxy().getReader().rawData.cursors;
        }
    },

    loadPage: function(page, options) {
        this.priorPage=this.currentPage;
        this.callParent(arguments);
    },
    
    sort: function() {
        this.priorPage=1;
        this.currentPage=1;
        this.cursors.current='';
        this.callParent(arguments);
    }
});