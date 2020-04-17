//------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Engine/Commons/EngineCommon.hpp"
#include "Engine/Core/EventSystems.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Math//AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include <vector>
#include <string>

class Game;
class TextureView;
class UIRadioGroup;
class BitmapFont;
class UIButton;

//------------------------------------------------------------------------------------------------------------------------------
enum eWidgetType
{
	UI_WIDGET,
	UI_BUTTON,
	UI_LABEL,
	UI_RADIO_GROUP,

	DEFAULT_WIDGET = UI_WIDGET
};

//------------------------------------------------------------------------------------------------------------------------------
class InputEvent
{
public:
	InputEvent(const std::string& name, EventArgs& args);
	~InputEvent();

	inline bool			WasConsumed() { return m_consumed; };

public:
	bool m_consumed = false;
	std::string m_name;
	std::string m_clickType;
	EventArgs m_args;

};

//------------------------------------------------------------------------------------------------------------------------------
class UIWidget
{
	friend class UIRadioGroup;

public:
	UIWidget(Game* game, UIWidget* parent);
	virtual ~UIWidget(); // virtual dctr - why?  Baseline this should kill all my children; 

	void					UpdateBounds(AABB2 const &container);
	void					ProcessInput(InputEvent &evt); // handles input - may consume the event (but it is still passed about to help update state)
	void					Render(); // assumes a camera has already been set; 
	void					RenderForWidgetType();
	void					RenderMouseData() const;

	void					SetColor(const Rgba& color);
	void					SetSize(const Vec4& size);
	void					SetPosition(const Vec4& position);
	void					SetPivot(const Vec2& pivoit);

	UIWidget*				AddChild(UIWidget *widget);
	void					RemoveChild(UIWidget *widget);

	void					SetRadioGroup(UIRadioGroup *group); // adds or removes me from a group

	void					SetWidgetType(eWidgetType widgetType);

	inline UIWidget*		GetParent() { return m_parent; }

	// accessors
	inline Vec2				GetWorldPosition() const { return m_position; }
	inline AABB2			GetWorldBounds() const { return m_worldBounds; }

	AABB2					GetWidgetDimensions(const AABB2& parentBounds);

	Vec2					GetRelativePosToParent(const Vec2& positionInWorld);

	// Function used to range map only for rendering
	Vec2					GetRelativePosToScreen(const Vec2& positionOnScreen);

public:
	// templated helper I like having
	template <typename T>
	T* CreateChild(const AABB2& bounds, const Vec4& size = Vec4(1.f, 1.f, 0.0f, 0.0f), const Vec4& position = Vec4(.5f, .5f, 0.0f, 0.0f), const Vec2& pivot = Vec2(.5f, .5f), const Rgba& color = Rgba::DARK_GREY)
	{
		T* child = new T(m_game, this);

		child->SetSize(size);
		child->SetPosition(position);
		child->SetPivot(pivot);
		child->SetColor(color);
		child->UpdateBounds(bounds);

		m_children.push_back(child);
		return child;
	}

protected:
	// helpers; 
	void					UpdateChildrenBounds(); // will update children using my bounds as their container
	void					ProcessChildrenInput(InputEvent &evt); // update input - process backwards through the list (things on top get first crack)
	void					RenderChildren();  // render children - process forward (later things show up on top)
	void					DestroyChildren(); // be sure to kill your children when you go... programming is dark; 

private:
	// heirarchy information
	UIWidget *m_parent = nullptr;
	Game* m_game = nullptr;
	std::vector<UIWidget*> m_children;

	// human settable independent variables; 
	Vec4 m_virtualPosition = Vec4(.5f, .5f, 0.0f, 0.0f);     // centered relative to container
	Vec4 m_virtualSize = Vec4(1.f, 1.f, 0.0f, 0.0f);   // stretch to 100% of the screen 
	Vec2 m_pivot = Vec2(.5f, .5f);                 // centered

	// derived from the above;
	Vec2 m_position;
	AABB2 m_worldBounds;

	// Misc Data
	//eUIWidgetFlags m_uiFlags   = 0U;       // some state tracking; 
	UIRadioGroup *m_radioGroup = nullptr;  // which radio group do I belong to?

	Rgba m_color = Rgba::WHITE;

	std::string	m_defaultShaderName = "default_unlit.00.hlsl";

	eWidgetType m_widgetType = DEFAULT_WIDGET;
	BitmapFont* m_font = nullptr;
};

//------------------------------------------------------------------------------------------------------------------------------
// A radio group allows for only a single item in the group 
// to be selected at a time - ie, mutually exclusive selection
class UIRadioGroup : public UIWidget
{
public:
	UIRadioGroup(Game* game, UIWidget* parent);
	~UIRadioGroup();

	void SetChildrenUnSelected();

public:
	std::vector<UIButton*> m_radioChildren;
};

//------------------------------------------------------------------------------------------------------------------------------
class UILabel : public UIWidget
{
public:
	UILabel(Game* game, UIWidget* parent);
	~UILabel();

	void	SetLabelText(const std::string& labelText);

public:
	std::string m_labelText = "";
};

//------------------------------------------------------------------------------------------------------------------------------
class Event
{
public:
	Event(std::string const &commandLine);
	~Event();

public:
	std::string m_name;
	EventArgs m_args;
};

//------------------------------------------------------------------------------------------------------------------------------
class UIButton : public UIWidget
{
public:
	UIButton(Game* game, UIWidget* parent);
	~UIButton();

	void SetOnClick(const std::string& onClickEvent);
	void SetButtonTexture(const std::string& texturePath);
	void SetOnHover(const std::string& onHoverEvent);
	void SetRadioType(bool radioType);

	void Click()
	{
		Event evt(m_eventOnClick);
		g_eventSystem->FireEvent(evt.m_name, evt.m_args);

		if (m_isRadioType)
		{
			UIRadioGroup* parent = dynamic_cast<UIRadioGroup*>(GetParent());

			parent->SetChildrenUnSelected();
			m_isSelected = true;
		}
	}

	void OnHover()
	{
		/*
		Event evt( m_eventOnHover );
		g_eventSystem->FireEvent( evt.m_name, evt.m_args );
		*/

		SetColor(hoverColor);
	}

	void OnUnHover()
	{
		/*
		Event evt( m_eventOnHover );
		g_eventSystem->FireEvent( evt.m_name, evt.m_args );
		*/

		SetColor(unHovercolor);
	}

public:
	std::string m_eventOnClick = "play map=level0.map";
	std::string m_eventOnHover = "play map=level0.map";
	TextureView* m_buttonTexture = nullptr;
	bool m_isRadioType = false;
	bool m_isSelected = false;

	Rgba hoverColor = Rgba(0.4f, 0.9f, 0.9f, 1.f);
	Rgba unHovercolor = Rgba::CLEAR;
};

//------------------------------------------------------------------------------------------------------------------------------
class UISlider : public UIWidget
{
	/*
	void Change( float value )
	{
		m_value = value;
		// whatever other updating needed for visual reasons;

		Event evt( m_eventOnChange );
		evt.m_args.Add( "value", m_value ); // add this AS A FLOAT
		evt.m_args.Add( "source", this );   //  who triggered this event;

		EventFire( evt );
	}
	*/

	std::string m_eventOnChange = "changeRadius";
};
