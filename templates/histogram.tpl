<script type="text/javascript">
function drawVisualization() {
	var data = new google.visualization.DataTable();
	data.addColumn('string', 'Counts');
	{{#COLUMNS}}
	data.addColumn('number', 'Doc type {{DOC_TYPE}}');
	{{/COLUMNS}}
	{{#ROW}}
	data.addRow([
		'{{INDEX}}',
		{{#COLUMN}}
		{{DOC_COUNTS}}
		{{#COLUMN_separator}}, {{/COLUMN_separator}}
		{{/COLUMN}}
	]);
	{{/ROW}}
	var chart = new google.visualization.ColumnChart(document.getElementById('{{NAME}}'));
	chart.draw(data, {width: 1500, height: 400,vAxis: {title: "Count"},hAxis: {title: "{{TITLE}}",}});
}
google.setOnLoadCallback(drawVisualization);
</script>
<div id="{{NAME}}" style="width: 1500px; height: 400px;"></div>
