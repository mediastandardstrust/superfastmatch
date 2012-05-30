Ext.define('Superfastmatch.view.PerformancePanel', {
    extend: 'Ext.panel.Panel',
    alias: 'widget.performancepanel',
    itemId: 'PerformancePanel',
    title: 'Performance',
    forceFit: true,
    layout: {
        align: 'stretch',
        type: 'vbox'
    },
    
    buildStore: function(name){
      var model=Ext.define('Superfastmatch.model.Performance-'+name, {
        extend: 'Ext.data.Model',
        proxy: {
          type: "memory",
          reader: {
            type: "json",
            root: "rows"
          }
        },
        statics:{
          getColumns: function(){
            columns=[];
            Ext.each(this.getFields(),function(field){
              if (!Ext.Array.contains(['id','Group'],field.name)){
                columns.push({header:field.name,dataIndex:field.name,renderer: function(v){return !Ext.isNumeric(v)?v:((v%1)===0)?Ext.util.Format.number(v,v>999?"0,00":"0"):Ext.util.Format.number(v,"0.0000");}});                
              }
            });
            return columns;
          }
        }
      });   
      return Ext.create("Ext.data.Store",{
        model: model,
        groupField: 'Group'
      });
    },
    
    buildItems: function(grids){
      return {
          items: Ext.Array.map(grids,function(grid){
            return {
              xtype: 'gridpanel',
              title: grid+'s',
              flex: 1,
              itemId: grid+'Grid',
              forceFit: true,
              columns: [],
              viewConfig: {
                  preserveScrollOnRefresh: true
              },
              features: [{ftype: 'grouping',groupHeaderTpl: '{name}', startCollapsed: false}],
              store: this.buildStore(grid)
            };
          },this)
        };
    },

    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems(['Document','Association','Posting']));
        me.callParent(arguments);
    }
});
