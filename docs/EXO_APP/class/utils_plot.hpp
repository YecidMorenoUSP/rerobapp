#pragma once
#include <imgui.h>
#include <imgui_plot2.hpp>

class PlotWindow {
public: 
	std::vector<ImGui::PlotItem> items;
	ImGui::PlotInterface SCOPE;
	std::string winName;
	PlotWindow(){}

	PlotWindow(std::string xlabel, std::string ylabel, std::string namePlot) {
		SCOPE.y_axis.minimum = -50;
		SCOPE.y_axis.maximum = 50;
		SCOPE.x_axis.minimum = 0;
		SCOPE.x_axis.maximum = 60;
		SCOPE.x_axis.label = xlabel;
		SCOPE.y_axis.label = ylabel;
		SCOPE.title = namePlot;
		winName = "";
	}

	void clearItems() {
		items.clear();
	}

	ImGui::PlotItem * addItem(std::string label) {
		ImGui::PlotItem item;
		item.label = label;
		items.push_back(item);
		return &item;
	}
	
	void addDataItem(ImGui::PlotItem * item, ImVec2 data) {
		item->data.push_back(data);
	}

	void show() {
		ImGui::Plot("SCOPE", SCOPE, items);
	}
	void showNewWindow() {
		ImGui::Begin(SCOPE.title.c_str());
			ImGui::Plot("SCOPE", SCOPE, items);
		ImGui::End();
	}
};