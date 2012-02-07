Ext.define('Superfastmatch.store.Documents', {
    extend: 'Ext.data.Store',
    requires: ['Superfastmatch.model.Document'],
    model: 'Superfastmatch.model.Document',
    storeId: 'Documents',
    pageSize: 10,
    remoteSort: true,

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
    
    onLoad: function(store,records,successful){
      if (successful){
        store.cursors=store.getProxy().getReader().rawData.cursors;
      }
    },
    
    beforeLoad: function(store,operation){
      if (operation.start==0){
        operation.start='';
      }
    },
    
    move: function(position){
      var me=this;
      if (me.cursors.hasOwnProperty(position)){
        me.load({
          start: me.cursors[position]
        });
      }else{
        me.load();
      }
    },
    
    moveFirst: function(){
      this.move('first');
    },
    
    moveLast: function(){
      this.move('last');
    },
    
    movePrevious: function(){
      this.move('previous');
    },
    
    moveNext: function(){
      this.move('next');
    }
});