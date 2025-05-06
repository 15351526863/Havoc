#pragma once
#include "../../utils/render.h"
#include "../input_manager.h"
#include "../config.h"
#include "../variables.h"
#include "controls/containers.h"
#include <memory>
#include <optional>

#define ACCENT2 Color(182,139,252)
#define ACCENT  Color(115,155,255)
#define BACKGROUND Color(20,20,20)
#define GROUP Color(25,25,25)
#define OUTLINE_LIGHT Color(55,55,55)
#define OUTLINE_DARK Color(10,10,10)
#define DIM_ELEMENT Color(50,50,50)
#define HOVERED_ELEMENT Color(120,120,120)
#define SELECTED_ELEMENT Color(200,200,200)
#define BAR_SIZE 90
#define MARGIN 20
#define PADDING 8
#define ITEM_HEIGHT 12
#define ANIMATION_SPEED 10*Interfaces::Globals->flFrameTime

struct FocusItem_t {
	CMenuItem* m_pItem{};
	Vector2D m_vecDrawPos{};
	float m_flFocusAnim{};
	bool m_bFrameAfterFocus{};
	bool m_bFadeOut{};
	void Update(CMenuItem* item);
};

namespace input {
	inline bool hovered(const Vector2D& p, const Vector2D& s) { return Inputsys::hovered(p, s); }
	inline bool pressed(int k) { return Inputsys::pressed(k); }
	inline bool down(int k) { return Inputsys::down(k); }
	inline Vector2D mouse_delta() { return Inputsys::MouseDelta; }
	inline Vector2D mouse_pos() { return Inputsys::MousePos; }
}

namespace Menu {
	void Render();
	void RenderElements();
	void Register();
	void HandleControls();
	void SetCursor(bool resize);
	void LerpToCol(Color& col, Color to);
	inline float mapNumber(float i, float iMin, float iMax, float oMin, float oMax) {
		return (i - iMin) / (iMax - iMin) * (oMax - oMin) + oMin;
	}
	inline bool m_bOpened{ true };
	inline bool m_bRendering{};
	inline float m_flAlpha{};
	inline FocusItem_t m_pFocusItem{};
	inline Vector2D m_vecDrawPos{};
	inline void* m_pPopup{};
	inline uint8_t m_iSelectedTab{};
	inline CMenuTab m_cTabs[5]{};
	inline Vector2D m_vecSize{ 800,590 };
	inline Vector2D m_vecPos{ 500,200 };
	inline bool m_bDraggingMenu{};
	inline bool m_bDraggingSize{};
}
