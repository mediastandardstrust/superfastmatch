Ext.Loader.setConfig({
    enabled: true,
    disableCaching: false
});

Ext.Loader.setPath('Ext.ux', 'app/ux');
Ext.Loader.setPath('Ext', 'extjs/src');

Ext.application({
    name: 'Superfastmatch',
    models: ['Fragment','Search','Document'],
    controllers: ['Searches','Documents'],
    requires: ['Superfastmatch.view.MainViewPort'],
    launch: function() {
        Ext.QuickTips.init();
        var cmp1 = Ext.create('Superfastmatch.view.MainViewPort', {
            renderTo: Ext.getBody()
        });
        cmp1.show();
    }
});
