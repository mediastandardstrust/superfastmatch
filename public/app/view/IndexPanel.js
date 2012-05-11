Ext.define('Superfastmatch.view.IndexPanel', {
    extend: 'Ext.grid.Panel',
    alias: 'widget.indexpanel',
    requires: ['Superfastmatch.model.Index'],
    itemId: 'IndexPanel',
    title: 'Index',
    forceFit: true,
    viewConfig: {
      stripeRows: false,
      getRowClass : function(rec,index,params,store){ 
        return ((rec.getIndex().get('hash')%2)==0)?'x-grid-row-alt':''
      }
    },
    columns: [
                {text:"Hash",flex: 1,renderer: function(v,m,rec,row,col,store){return this.isSame(rec,row,store)?'':'<b>'+rec.getIndex().get('hash')+'</b>';}},
                {text:"Bytes",flex: 1,renderer: function(v,m,rec,row,col,store){return this.isSame(rec,row,store)?'':'<b>'+rec.getIndex().get('bytes')+'</b>';}},
                {text:"DocType",flex: 1,dataIndex:'doctype'},
                {text:"Bytes",flex: 1,dataIndex:'bytes'},
                {text:"Doc IDs",flex: 8,dataIndex:'docids'},
                {text:"Deltas",flex: 8,dataIndex:'deltas'}
              ],
    
    buildToolBar: function(store){
      return {
          xtype: 'pagingtoolbar',
          itemId: 'IndexPaging',
          store: store,
          dock: 'bottom',
          displayInfo: true    
      }
    },
    
    isSame: function(rec,row,store){
      return (row==0)?false:store.getAt(row-1).getIndex().get('hash')==rec.getIndex().get('hash');
    },
    
    loadIndex: function(store,records,success){
      var doctypes=Ext.create('Ext.data.Store',{
        model:'Superfastmatch.model.IndexRow'
      });
      store.each(function(rec){
        doctypes.add(rec.doctypes().data.items);
      });
      this.reconfigure(doctypes);
    },
      
    initComponent: function() {
        var me = this,
            store=Ext.create('Ext.data.Store',{
              model: 'Superfastmatch.model.Index',
              listeners: {
                load: me.loadIndex,
                scope: me
              }
            });
        me.callParent(arguments);
        me.addDocked(me.buildToolBar(store));
    }
});
