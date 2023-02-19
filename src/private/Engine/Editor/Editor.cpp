#include "Engine/Editor/Editor.h"

Editor* Editor::instance;

Editor::Editor() {

}

void Editor::RenderMenu() {
	ImGui::BeginMainMenuBar();
	ImGui::MenuItem("File");
	ImGui::EndMainMenuBar();
}

void Editor::Render() {
	this->RenderMenu();
}

Editor* Editor::GetInstance() {
	if (Editor::instance == nullptr)
		Editor::instance = new Editor();
	return Editor::instance;
}