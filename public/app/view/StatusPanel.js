Ext.define('Superfastmatch.view.StatusPanel', {
    extend: 'Ext.panel.Panel',
    alias: 'widget.statuspanel',
    itemId: 'StatusPanel',
    title: 'Status',
    forceFit: true,
    layout: {
        align: 'stretch',
        type: 'vbox'
    },
        
    buildStore: function(root,fields){
      return Ext.create("Ext.data.Store",{
        fields: fields,
        proxy: {
          type: "memory",
          reader: {
            type: "json",
            root: root
          }
        }
      });
    },
    
    buildItems: function(){
      return {
          items: [
              {
                  xtype: 'container',
                  layout: {
                      align: 'stretch',
                      type: 'hbox'
                  },
                  flex: 1,
                  items: [
                            {
                              xtype: 'gridpanel',
                              title: 'Databases',
                              itemId: 'DatabaseGrid',
                              flex: 2,
                              forceFit: true,
                              columns: [
                                {text:"Name",dataIndex: "name"},
                                {text:"Count",dataIndex: "count"},
                                {text:"Size",dataIndex: "size", renderer: Ext.util.Format.fileSize},
                                {text:"Path",dataIndex: "path"},
                              ],
                              store: this.buildStore("dbs",["name","count","size","path"])
                            },
                            {
                              xtype: 'propertygrid',
                              title: 'Configuration',
                              itemId: 'ConfigurationGrid',
                              flex: 1,
                              source: {}
                            },
                            {
                              xtype: 'propertygrid',
                              title: 'Stats',
                              itemId: 'StatsGrid',
                              flex: 1,
                              source: {}
                            }
                            
                          ]
              },{
                xtype: 'container',
                layout: {
                    align: 'stretch',
                    type: 'hbox'
                },
                flex: 1,
                items: [
                  {
                    xtype: 'gridpanel',
                    title: 'Slots',
                    flex: 1,
                    itemId: 'SlotGrid',
                    forceFit: true,
                    columns: [
                      {text:"Number",dataIndex:"slot_number"},
                      {text:"Hash Count",dataIndex:"hash_count"}
                    ],
                    store: this.buildStore("slots",["slot_number","hash_count"])
                  },
                  {
                    xtype: 'chart',
                    title: 'Histogram',
                    flex: 1,
                    itemId: 'Histogram',
                    axes: [
                        {
                            title: 'Count',
                            type: 'Numeric',
                            position: 'bottom',
                            fields: ['count'],
                            minimum: 0,
                            maximum: 500
                        },
                        {
                            title: 'Occurrences',
                            type: 'Numeric',
                            position: 'left',
                            fields: ['occurrences'],
                            minimum: 0,
                            maximum: 100
                        }
                    ],
                    series: [{
                      type: "column",
                      axis: "left",
                      xField: "count",
                      yField: "occurrences"
                    }],
                    store: this.buildStore("histogram",["count","occurrences"])
                  }
                ]
              }
          ]
        }
    },

    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
    }
});
