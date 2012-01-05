Ext.define('Ext.ux.panel.StatefulTabPanel', {
    extend: 'Ext.TabPanel',
    alias: 'widget.statefultab',
    
    stateEvents: ['tabchange'],
    getState: function() {return{tab:this.getActiveTab().id}},
    applyState: function(state) {this.setActiveTab(state.tab);}
});

