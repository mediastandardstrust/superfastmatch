{{>HEADER}}
	<h1>Index</h1>
	<table>
		<thead>
			<th>Hash</th>
			<th>Document Type</th>
			<th>Document ID's</th>
			<th>Document ID Delta's</th>
		</thead>
		<tbody>
		{{#POSTING}}
			<tr>
				{{#HASH}}
				<td rowspan="{{DOC_TYPE_COUNT}}">{{HASH}}</td>
				{{/HASH}}
				<td class="line">{{DOC_TYPE}}</td>
				<td class="line">
					{{#DOC_IDS}}
					{{DOC_ID}}{{#DOC_IDS_separator}}, {{/DOC_IDS_separator}}
					{{/DOC_IDS}}
				</td>
				<td class="line">
					{{#DOC_DELTAS}}
					{{DOC_DELTA}}
					{{#DOC_DELTAS_separator}}, {{/DOC_DELTAS_separator}}
					{{/DOC_DELTAS}}
				</td>
			</tr>
		{{/POSTING}}
		</tbody>
	<table>
{{>FOOTER}}