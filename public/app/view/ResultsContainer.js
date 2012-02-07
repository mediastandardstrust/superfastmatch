Ext.define('Superfastmatch.view.ResultsContainer', {
    extend: 'Ext.container.Container',
    alias: 'widget.resultscontainer',
    requires: ['Superfastmatch.store.Fragments'],

    itemId: 'Results',
    layout: {
       align: 'stretch',
       type: 'vbox'
    },

    fragmentStore: undefined,
    currentSearch: undefined,

    buildItems: function(){
        return {
            items: [Ext.create('Ext.grid.Panel',{
                        itemId: 'Documents',
                        title: 'Matching Documents',
                        forceFit: true,
                        flex: 1,
                        viewConfig: {
                            itemId: 'DocumentsView',
                            emptyText: 'No Matching Documents Found'
                        },
                        columns: [],
                        selModel: Ext.create('Ext.selection.RowModel', {
                            allowDeselect: true,
                            mode: 'SINGLE'
                        })              
                    }),
                    Ext.create('Ext.grid.Panel',{
                        itemId: 'Fragments',
                        title: 'Fragments (All Documents)',
                        forceFit: true,
                        loadMask: true,
                        flex: 2,
                        columns: [
                            {
                                xtype: 'gridcolumn',
                                itemId: 'FragmentTextColumn',
                                dataIndex: 'from',
                                flex: 8,
                                text: 'Text',
                                renderer: function(val,meta,record){
                                    return '<pre class="wrap">'+ record.get('text') +'</pre>';
                                },
                            },
                            {
                                xtype: 'numbercolumn',
                                dataIndex: 'length',
                                flex: 1,
                                text: 'Length',
                                format: '0,000'
                            },
                            {
                                xtype: 'numbercolumn',
                                dataIndex: 'count',
                                flex: 1,
                                text: 'Count',
                                format: '0,000'
                            },
                            {
                                xtype: 'numbercolumn',
                                dataIndex: 'docCount',
                                flex: 1,
                                text: 'Documents',
                                format: '0,000'
                           }
                        ],
                        viewConfig: {
                            itemId: 'FragmentView',
                            emptyText: 'No Fragments Found',
                            loadMask: false
                        },
                        disableSelection: true
                    })
            ]
        };
    },
    
    buildActions: function(){
      var me=this;
      return {xtype:'actioncolumn',header: 'Actions',minWidth:50,maxWidth:50,items: [{
            handler:function (grid, rowIndex, colIndex) {
                var rec = grid.getStore().getAt(rowIndex);
                me.fireEvent('showdocument',{doctype:rec.get('doctype'),docid:rec.get('docid')});
            },
            getClass: function(){return 'icon-go';},
            tooltip: 'Go to Document'
          },{
            handler:function (grid, rowIndex, colIndex) {
                var rec = grid.getStore().getAt(rowIndex);
                me.fireEvent('comparedocument',{search: me.currentSearch,match: rec});
            },
            getClass: function(){return 'icon-side';},
            tooltip: 'View Documents side-by-side'
          }]
      }
    },
    
    initComponent: function() {
        var me = this;
        Ext.applyIf(me,me.buildItems());
        me.callParent(arguments);
        me.fragmentStore=Ext.create('Superfastmatch.store.Fragments');
        me.down('#Fragments').addDocked(Ext.create('Ext.toolbar.Paging',{
            itemId: 'FragmentPaging',
            displayInfo: true,
            dock: 'bottom',
            store: me.fragmentStore
        }));
        me.down('#Fragments').reconfigure(me.fragmentStore);
        me.down('#FragmentPaging').bindStore(me.fragmentStore,true);
        me.down('#Documents').on('selectionchange',me.filterFragments,me);
        me.down('#Fragments').on({
           itemmouseenter: me.highlightFragment,
           itemmouseleave: me.unHighlightFragment,
           scope:          me
        });
        me.addEvents('highlightchange','showdocument','comparedocument');
    },
    
    loading: function(){
        this.down('#Documents').setLoading(true);
        this.down('#Fragments').setLoading(true);
    },
    
    loadMatches: function(search){
        var me=this;
            documents=me.down('#Documents'),
            fragments=me.down('#Fragments'),
            paging=me.down('#FragmentPaging'),
            records=[],
            columns=search.documents().model.getColumns().concat(me.buildActions());
        me.currentSearch=search;
        documents.reconfigure(search.documents(),columns);
        me.enable();
        me.fragmentStore.loadDocuments(search.documents().data.items);
        paging.moveFirst();
        documents.setLoading(false);
        fragments.setLoading(false);
    },
    
    clearMatches: function(){
        var me=this;
            documents=me.down('#Documents'),
        documents.getStore().removeAll();
    },

    highlightFragment: function(view,record){
        var me=this,
            documents=me.down('#Documents');
            currentDocument=record.get('documents'),
            start=record.get('from'),
            length=record.get('length'),
            text=me.currentSearch.get('text').substr(start,length);
        me.fireEvent('highlightchange',{action:'enter',text: text,start: start,length: length});
        documents.getStore().filterBy(function(record,id){
            return currentDocument.hasOwnProperty(record.get('doctype')+':'+record.get('docid'));
        })
        documents.setTitle('Matching Documents ("'+Ext.String.ellipsis(record.get('text'),100,true)+'...")');
    },
    
    unHighlightFragment: function(view,record){
        var me=this,
            documents=me.down('#Documents'),
            start=record.get('from'),
            lengh=record.get('length'),
            text=me.currentSearch.get('text').substr(start,length);
        me.fireEvent('highlightchange',{action:'leave',text: text,start: start,length: length});
        documents.getStore().clearFilter();
        documents.setTitle('Matching Documents');
    },

    filterFragments: function(selmodel,selected){
        var me=this,
            fragments=me.down('#Fragments');
        if (selected.length==0){
            me.fragmentStore.clearFilter();
            fragments.setTitle('Fragments (All Documents)');
        }else{
            var doc=selected[0].get('doctype')+':'+selected[0].get('docid');
            me.fragmentStore.remoteFilter=false;
            me.fragmentStore.clearFilter();
            me.fragmentStore.remoteFilter=true;
            me.fragmentStore.filter([{
                filterFn:function(record){
                    return record.get('documents').hasOwnProperty(doc);
                }
            }]);
            fragments.setTitle('Fragments ('+doc+')');
        }
    } 
});