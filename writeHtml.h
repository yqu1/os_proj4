#ifndef WRITEHTML_H
#define WRITEHTML_H
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

void writeHtml(int numfetch) {
    ofstream file;
    ostringstream ss;
    ss << numfetch + 1;
    string n = ss.str();
    file.open("summary.html");
    string content = "<!DOCTYPE html><meta charset=\"utf-8\"><style> body { font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; width: 960px; height: 500px; position: relative; } svg { width: 100%; height: 100%; position: center; } .toolTip { font-family: \"Helvetica Neue\", Helvetica, Arial, sans-serif; position: absolute; display: none; width: auto; height: auto; background: none repeat scroll 0 0 white; border: 0 none; border-radius: 8px 8px 8px 8px; box-shadow: -3px 3px 15px #888888; color: black; font: 12px sans-serif; padding: 5px; text-align: center; } text { font: 10px sans-serif; color: white; } text.value { font-size: 120%; fill: white; } .axisHorizontal path{ fill: none; } .axisHorizontal .tick line { stroke-width: 1; stroke: rgba(0, 0, 0, 0.2); } .bar { fill: steelblue; fill-opacity: .9; }</style><body><script src=\"http://d3js.org/d3.v3.min.js\"></script><script src = \"https://cdnjs.cloudflare.com/ajax/libs/underscore.js/1.8.3/underscore-min.js\"></script><script> var i = 1; function createGraphs(i , n) { if(n == 1) return; d3.csv(i + \".csv\", function(d) { return { Time: d.Time, Phrase: d.Phrase, Site: d.Site, Count: +d.Count }; }, function(t) { var temp = _.groupBy(t, function(d) {return d.Site}); d3.select(\"body\").append(\"h1\").text(\"Batch: \" + i + \".csv\"); console.log(temp); for(key in temp) { var data = temp[key]; var div = d3.select(\"body\").append(\"div\").attr(\"class\", \"toolTip\"); var axisMargin = 20, margin = 40, valueMargin = 4, width = parseInt(d3.select('body').style('width'), 10), height = parseInt(d3.select('body').style('height'), 10), barHeight = (height-axisMargin-margin*2)* 0.4/data.length, barPadding = (height-axisMargin-margin*2)*0.6/data.length, data, bar, svg, scale, xAxis, labelWidth = 0; max = d3.max(data, function(d) { return d.Count; }); svg = d3.select('body') .append(\"svg\") .attr(\"width\", width) .attr(\"height\", height); bar = svg.selectAll(\"g\") .data(data) .enter() .append(\"g\"); bar.attr(\"class\", \"bar\") .attr(\"cx\",0) .attr(\"transform\", function(d, i) { return \"translate(\" + margin + \",\" + (i * (barHeight + barPadding) + barPadding) + \")\"; }); bar.append(\"text\") .attr(\"class\", \"label\") .attr(\"y\", barHeight / 2) .attr(\"dy\", \".35em\") .text(function(d){ return d.Phrase; }).each(function() { labelWidth = Math.ceil(Math.max(labelWidth, this.getBBox().width)); }); scale = d3.scale.linear() .domain([0, max]) .range([0, width - margin*2 - labelWidth]); xAxis = d3.svg.axis() .scale(scale) .tickSize(-height + 2*margin + axisMargin) .orient(\"bottom\"); bar.append(\"rect\") .attr(\"transform\", \"translate(\"+labelWidth+\", 0)\") .attr(\"height\", barHeight) .attr(\"width\", function(d){ return scale(d.Count); }); bar.append(\"text\") .attr(\"class\", \"value\") .attr(\"y\", barHeight / 2) .attr(\"dx\", -valueMargin + labelWidth) .attr(\"dy\", \".35em\") .attr(\"text-anchor\", \"end\") .text(function(d){ return (d.Count); }) .attr(\"x\", function(d){ var width = this.getBBox().width; return Math.max(width + valueMargin, scale(d.Count)); }); bar .on(\"mousemove\", function(d){ div.style(\"left\", d3.event.pageX+10+\"px\"); div.style(\"top\", d3.event.pageY-25+\"px\"); div.style(\"display\", \"inline-block\"); div.html((d.Phrase)+\"<br>\"+(d.Count)); }); bar .on(\"mouseout\", function(d){ div.style(\"display\", \"none\"); }); svg.insert(\"g\",\":first-child\") .attr(\"class\", \"axisHorizontal\") .attr(\"transform\", \"translate(\" + (margin + labelWidth) + \",\"+ (height - axisMargin - margin)+\")\") .call(xAxis); svg.append(\"text\") .attr(\"x\", (width / 2)) .attr(\"y\", 30) .attr(\"text-anchor\", \"middle\") .style(\"font-size\", \"30px\") .text(\"Search Results for \" + key); } i += 1; console.log(i); if(i < n) { createGraphs(i, n); } }); } createGraphs(i, " + n + "); </script></body>";
    file << content;
    file.close();
}

#endif

