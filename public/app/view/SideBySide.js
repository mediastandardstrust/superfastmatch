Ext.define('Superfastmatch.view.SideBySide', {
    extend: 'Ext.window.Window',
    requires: ['Superfastmatch.model.Search','Superfastmatch.store.Fragments'],
    layout: {
        type: 'vbox',
        align: 'stretch'
    },
    maximized: true,
    preventHeader: true,
    
    buildItems: function(){
      var me=this;
      return {
        items: [{
                  xtype: 'container',
                  layout:{
                    type: 'hbox',
                    align: 'stretch'
                  },
                  flex:2,
                  items:[{
                    xtype: 'panel',
                    title: 'Left',
                    tpl: Ext.create('Ext.XTemplate','<pre class="wrap">{text}</pre>'),
                    itemId: 'LeftText',
                    autoScroll: true,
                    flex: 1
                  },{
                    xtype:'panel',
                    title: 'Right',
                    tpl: Ext.create('Ext.XTemplate','<pre class="wrap">{text}</pre>'),
                    tools: [{
                      type: 'close',
                      tooltip: 'Close Side By Side View',
                      handler: function(){me.close();}
                    }],
                    itemId: 'RightText',
                    autoScroll: true,
                    flex: 1
                  }]
                },{
                  xtype: 'grid',
                  itemId: 'Fragments',
                  columns:[
                            {
                                xtype: 'gridcolumn',
                                itemId: 'FragmentTextColumn',
                                dataIndex: 'from',
                                flex: 8,
                                text: 'Text',
                                renderer: function(val,meta,record){
                                    return '<pre class="wrap">'+ record.get('text') +'</pre>';
                                },
                            }
                          ],
                  title: 'Fragments',
                  store: Ext.create('Superfastmatch.store.Fragments'),
                  flex:1
               }]
      };
    },
    
    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
    },
    
    compare: function(evArgs){
      var me=this,
          left=me.down('#LeftText')
          right=me.down('#RightText'),
          fragments=me.down('#Fragments'),
          search=Ext.ModelManager.getModel('Superfastmatch.model.Search'),
          id=evArgs.match.get('doctype')+'/'+evArgs.match.get('docid')+'/';
      left.update({text:evArgs.search.get('text')});
      me.show();
      right.setLoading(true);
      search.load(null,{
        url: '/document/'+id,
        success: function(search){
          right.update({text:search.get('text')});
          right.setLoading(false);
        }
      });
      fragments.getStore().loadDocuments(evArgs.match);
      fragments.getStore().load();
    } 
});