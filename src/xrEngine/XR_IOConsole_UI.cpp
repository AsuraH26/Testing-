#include "stdafx.h"
#include "XR_IOConsole.h"
#include "xr_ioc_cmd.h"

void CConsole::DrawUIConsoleVars()
{
	if (!Engine.External.EditorStates[static_cast<std::uint8_t>(EditorUI::CmdVars)]) {
		return;
	}

	if (!ImGui::Begin("DebugConsoleVars", &Engine.External.EditorStates[static_cast<std::uint8_t>(EditorUI::CmdVars)])) {
		ImGui::End();
		return;
	}

	for (const auto& [Name, Command] : Commands) {
		if (auto Mask = dynamic_cast<CCC_Mask*>(Command)) {
			continue;
		}

		if (auto Boolean = dynamic_cast<CCC_Boolean*>(Command)) {
			if (ImGui::Checkbox(Boolean->Name(), Boolean->value)) {
				Boolean->Execute(Boolean->value ? "1" : "0");
			}
			continue;
		}

		if (auto Float = dynamic_cast<CCC_Float*>(Command)) {
			float test = Float->GetValue();
			if (ImGui::SliderFloat(Float->Name(), &test, Float->min, Float->max)) {
				string32 String = {};
				xr_sprintf(String, "%3.5f", test);
				Float->Execute(String);
			}

			continue;
		}

		if (auto Integer = dynamic_cast<CCC_Integer*>(Command)) {
			int test = Integer->GetValue();
			if (ImGui::SliderInt(Integer->Name(), &test, Integer->min, Integer->max)) {
				string32 String = {};
				xr_sprintf(String, "%i", test);
				Integer->Execute(String);
			}

			continue;
		}

		if (auto Token = dynamic_cast<CCC_Token*>(Command)) {
			int Id = (int)*Token->value;
			xr_token* tok = Token->GetToken();

			const char* Value = "?";
			while (tok->name) {
				if (tok->id == Id) {
					Value = tok->name;
					break;
				}
				tok++;
			}

			if (ImGui::BeginCombo(Token->Name(), Value)) {
				int Id = (int)*Token->value;
				xr_token* tok = Token->GetToken();
				while (tok->name) {
					if (ImGui::Selectable(tok->name, tok->id == Id)) {
						Token->Execute(tok->name);
					}
					tok++;
				}
				ImGui::EndCombo();
			}
			continue;
		}

		if (auto Vector = dynamic_cast<CCC_Vector3*>(Command)) {
			auto& Val = *Vector->GetValuePtr();
			if (ImGui::SliderFloat3(Vector->Name(), &Val.x, Vector->min.x, Vector->max.x)) {
				string64 str = {};
				xr_sprintf(str, sizeof(str), "(%f, %f, %f)", Val.x, Val.y, Val.z);
				Vector->Execute(str);
			}

			continue;
		}
	}
	ImGui::End();
}

void CConsole::DrawUIConsole()
{
	constexpr u32 MaxHintCommands = 5;
	char InputBuf[256] = {};

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
	if (!Engine.External.EditorStates[static_cast<std::uint8_t>(EditorUI::CmdConsole)]) {
		return;
	}

	if (!ImGui::Begin("DebugConsole", &Engine.External.EditorStates[static_cast<std::uint8_t>(EditorUI::CmdConsole)], ImGuiWindowFlags_NoDecoration)) {
		ImGui::End();
		return;
	}

	if (ImGui::BeginChild("DebugConsoleScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar)) {
		if (!LogFile->empty()) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
			
			const float TextYSize = ImGui::CalcTextSize(((*LogFile)[0]).c_str()).y;
			const int MaxTextCount = (ImGui::GetContentRegionAvail().y / TextYSize) + 1;

			int CursorPos = std::max((int)(LogFile->size() - MaxTextCount - scroll_delta), 0);
			for (int i = CursorPos; i < LogFile->size(); i++) {
				LPCSTR ls = ((*LogFile)[i]).c_str();
				if (!ls) {
					continue;
				}

				ImGui::TextUnformatted(ls);
			}

			if (scroll_delta == 0) {
				ImGui::SetScrollHereY(1.0f);
			}

			ImGui::PopStyleVar();
		}
	}

	ImGui::EndChild();
	ImGui::End();
}