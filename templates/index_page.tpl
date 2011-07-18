{{>HEADER}}
	<h1>Index</h1>
	<table class="wide">
		<caption>{{>PAGING}}</caption>
		<thead>
			<th>Hash</th>
			<th>Bytes</th>
			<th>Document Type</th>
			<th>Document ID's</th>
			<th>Document ID Delta's</th>
		</thead>
		<tbody>
		{{#POSTING}}
			<tr>
				{{#HASH}}
				<td rowspan="{{DOC_TYPE_COUNT}}">{{HASH}}</td>
				<td rowspan="{{DOC_TYPE_COUNT}}">{{BYTES}}</td>
				
				{{/HASH}}
				<td class="line"><a href="/document/{{DOC_TYPE}}/">{{DOC_TYPE}}</a></td>
				<td class="line">
					{{#DOC_IDS}}<a href="/document/{{DOC_TYPE}}/{{DOC_ID}}/">{{DOC_ID}}</a>{{#DOC_IDS_separator}}, {{/DOC_IDS_separator}}{{/DOC_IDS}}
				</td>
				<td class="line">
					{{#DOC_DELTAS}}{{DOC_DELTA}}{{#DOC_DELTAS_separator}}, {{/DOC_DELTAS_separator}}{{/DOC_DELTAS}}
				</td>
			</tr>
		{{/POSTING}}
		</tbody>
	<table>		
{{>FOOTER}}