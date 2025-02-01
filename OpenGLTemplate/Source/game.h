#pragma once

#include "Includes.h"

static uintptr_t g_base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
constexpr auto text_section_hash = HASH(".text");

struct hook_t
{

	const char* m_symbol{};
	void* m_target_address{};
	uintptr_t m_original_address{};
	void* m_callback{};

	template <typename fn_t, typename...fn_args>
	auto call_original(fn_args&&... args) { return reinterpret_cast<fn_t>(m_original_address)(std::forward<fn_args>(args)...); }

};

class c_hook_manager
{

private:

	std::vector<hook_t> m_hooks{};

public:

	void add_hook(uintptr_t target_address, void* callback, const char* symbol) { m_hooks.push_back({ symbol, reinterpret_cast<void*>(target_address), NULL, callback}); }
	void add_hook_from_signature(void* callback, const char* symbol, const char* signature) { m_hooks.push_back({ symbol, reinterpret_cast<void*>(find_signature(g_base, signature, text_section_hash)), NULL, callback}); }

	hook_t get_hook(const char* symbol)
	{

		for (auto& hook : m_hooks)
			if (!strcmp(hook.m_symbol, symbol))
				return hook;

		return {};

	}

	void create_hooks()
	{

		for (const auto& hook : m_hooks)
		{

			if (MH_CreateHook(hook.m_target_address, hook.m_callback, (void**)(&hook.m_original_address)) != MH_OK)
				printf("[!] \"%s\" failed!\n", hook.m_symbol);
			else
				printf("[!] \"%s\" success!\n", hook.m_symbol);

		}

	}

}; static c_hook_manager g_hook_manager{};

#define INSTANCE_STEP_SYMBOL "__int64 __fastcall Character::instanceStep(Character *this, float a2)"
#define INSTANCE_STEP_SIGNATURE "41 57 41 56 41 55 41 54 55 57 56 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 44 0F 29 8C 24 ? ? ? ? 44 0F 29 94 24 ? ? ? ? 44 0F 29 9C 24 ? ? ? ? 44 0F 29 A4 24 ? ? ? ? 48 83 B9"

#define CLAMP_SPEED_SYMBOL "__int64 __fastcall Character::clampSpeed(Character *this, float a2)"
#define CLAMP_SPEED_SIGNATURE "48 8B 01 48 03 48 ? F3 0F 10 51"

#define GET_MOVE_SPEED_SYMBOL "float __fastcall CharacterBase::movespeed(CharacterBase *this)"
#define GET_MOVE_SPEED_SIGNATURE "F3 0F 10 05 ? ? ? ? 80 B9"

#define CONSUME_MANA_SYMBOL "__int64 __fastcall AliveObject::consumeMana(AliveObject *this, float amount)"
#define CONSUME_MANA_SIGNATURE "31 C0 F3 0F 10 41 ? 0F 2F C8 77 ? F3 0F 5C C1 B8 ? ? ? ? F3 0F 11 41 ? C3 90 0F 1F 40 ? 41 54"

#define CONSUME_STAMINA_SYMBOL "__int64 __fastcall AliveObject::consumeStamina(AliveObject *this, float a2)"
#define CONSUME_STAMINA_SIGNATURE "31 C0 F3 0F 10 41 ? 0F 2F C8 77 ? F3 0F 5C C1 B8 ? ? ? ? F3 0F 11 41 ? C3 90 0F 1F 40 ? 48 8B 01"

#define RECEIVE_RAW_DAMAGE_SYMBOL "__int64 __fastcall AliveObject::receiveRawDamages(AliveObject *this, int a2)"
#define RECEIVE_RAW_DAMAGE_SIGNATURE "66 0F EF C9 F3 0F 10 41 ? F3 0F 2A CA"

#define UPDATE_CAMERA_SYMBOL "void __fastcall centauri::GameState::updateCamera(centauri::GameState *this, WorldContext *a2, float a3)"
#define UPDATE_CAMERA_SIGNATURE "41 57 41 56 41 55 41 54 55 57 56 53 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 48 89 D3"

namespace game
{

	struct persistent_player_data_t
	{

	};

	struct character_base_t
	{

		persistent_player_data_t* get_player_data() { return reinterpret_cast<persistent_player_data_t*>((uintptr_t)this + 0x79); }

		__int64 __fastcall try_use_dash()
		{

			static auto try_use_dash_ptr = find_signature(g_base, "41 55 41 54 56 53 48 83 EC ? 48 89 CB E8 ? ? ? ? 84 C0", text_section_hash);

			return reinterpret_cast<__int64(__fastcall*)(character_base_t*)>(try_use_dash_ptr)(this);

		}

	};

	struct character_t
	{

		char m_pad[0x8]{};
		character_base_t* m_character_base{};

	};

	struct bounds_t { __int64 x1, y1, x2, y2; };

	struct rect_t
	{

		__int64 x1, y1, x2, y2;

		__int64 get_x1() { return *reinterpret_cast<__int64*>(reinterpret_cast<uintptr_t>(this) + 8); }
		__int64 get_y1() { return *reinterpret_cast<__int64*>(reinterpret_cast<uintptr_t>(this) + 12); }

		__int64 get_x2() { return *reinterpret_cast<__int64*>(reinterpret_cast<uintptr_t>(this) + 16); }
		__int64 get_y2() { return *reinterpret_cast<__int64*>(reinterpret_cast<uintptr_t>(this) + 20); }

		bounds_t get_bounds() { return { get_x1() << 32, get_y1() << 32, get_x2() << 32, get_y2() << 32 }; }

		unsigned __int64 __fastcall center()
		{
			__m128i v1; // xmm1
			__m128i v2; // xmm0

			// Use intrinsics to load values properly
			v1 = _mm_cvtsi32_si128(*((unsigned int*)this + 3)); // Safe load into __m128i
			v2 = _mm_cvtsi32_si128(*((unsigned int*)this + 2));

			// Calculate the center
			*(float*)&v1.m128i_i32[0] = (*(float*)&v1.m128i_i32[0] + *((float*)this + 5)) * 0.5f;
			*(float*)&v2.m128i_i32[0] = (*(float*)&v2.m128i_i32[0] + *((float*)this + 4)) * 0.5f;

			// Combine the two 32-bit results into a 64-bit value
			return ((unsigned __int64)_mm_cvtsi128_si32(v1) << 32) | (unsigned int)_mm_cvtsi128_si32(v2);
		}

	};

	struct invincible_t
	{

		__int64 set_invincible(double time)
		{

			static auto set_invincible_ptr = find_signature(g_base, "53 48 83 EC ? 0F 28 C1", text_section_hash);

			return reinterpret_cast<__int64(__fastcall*)(invincible_t*, double)>(set_invincible_ptr)(this, time);

		}

	};

	struct view_t
	{

		float __fastcall bottom()
		{
			return *((float*)this + 3) + *((float*)this + 5);
		}

		float __fastcall right()
		{
			return *((float*)this + 2) + *((float*)this + 4);
		}

		float __fastcall top()
		{
			return *((float*)this + 3);
		}

		float __fastcall left()
		{
			return *((float*)this + 2);
		}

	};

	struct alive_object_t
	{

		bool __fastcall get_health() { return *((float*)this + 7); }

		bool __fastcall is_alive() { return get_health() > 0.f; }
		
		DECLSPEC_NOINLINE rect_t get_bounds()
		{

			static auto generic_object_get_bounds = find_signature(g_base, "41 55 41 54 53 48 83 EC ? 48 8B 02 48 89 D3 49 89 CC", text_section_hash);

			rect_t bounds_rect{};

			auto return_rect = reinterpret_cast<rect_t*(__fastcall*)(rect_t*, float*)>(generic_object_get_bounds)(&bounds_rect, (float*)((char*)this + *(uintptr_t*)(*(uintptr_t*)this - 32i64)));

			return bounds_rect;

		}

		__int64 __fastcall get_game()
		{

			__int64 result; // rax

			result = *(__int64*)(*((__int64*)this + 5) + 368i64);
			return result;

		}

		view_t* get_view()
		{

			auto v3 = (view_t*)(*(__int64*)((alive_object_t*)((char*)this + *(__int64*)(*(__int64*)this - 32i64)))->get_game()
				+ 1456i64);

			return v3;

		}

		invincible_t* get_invincible() { return reinterpret_cast<invincible_t*>(reinterpret_cast<uintptr_t>(this) + 0x1A); }

	};

	struct game_t
	{

		auto height() { return (unsigned int)(*((DWORD*)this + 9) << 7); }
		auto width() { return (unsigned int)(*((DWORD*)this + 8) << 7); }

	};

	struct world_context_t
	{

		auto get_objects_begin() { return *(game::alive_object_t***)((uintptr_t)this + 0x270); }
		auto get_objects_end() { return *(game::alive_object_t***)((uintptr_t)this + 0x278); }

		auto get_game() { return (game_t*)((uintptr_t)this + 0x1A); }

	};

	struct game_state_t
	{

		float get_zoom()
		{

			auto v2 = (__int64*)*((__int64*)this + 1);
			auto v3 = *((__int64*)v2 + 1);

			auto graphics_settings = (__int64*)(v3 + 32);

			return *(float*)(graphics_settings + 5);

		}

	};

}

namespace character
{

	__int64 __fastcall hk_instance_step(game::character_t* thisptr, float a2)
	{

		static auto this_hook = g_hook_manager.get_hook(INSTANCE_STEP_SYMBOL);

		return this_hook.call_original<decltype(&hk_instance_step)>(thisptr, a2);

	}

	__int64 __fastcall hk_clamp_speed(game::character_t* thisptr, float max_speed)
	{

		static auto this_hook = g_hook_manager.get_hook(CLAMP_SPEED_SYMBOL);

		return this_hook.call_original<decltype(&hk_clamp_speed)>(thisptr, max_speed);

	}

}

namespace character_base
{

	float __fastcall hk_move_speed(game::character_base_t* thisptr)
	{

		static auto this_hook = g_hook_manager.get_hook(GET_MOVE_SPEED_SYMBOL);

		// __int64 __fastcall CharDashState::step(CharDashState *this, CharacterBase *a2, float a3)
		static auto dash_move_speed_return_address = find_signature(g_base, "E8 ? ? ? ? F3 44 0F 10 94 24 ? ? ? ? F3 44 0F 59 C0", text_section_hash) + 5;

		if (_ReturnAddress() == (void*)dash_move_speed_return_address)
			return this_hook.call_original<decltype(&hk_move_speed)>(thisptr);
		else
			return 8.f;

	}

}

namespace alive_object
{

	__int64 __fastcall hk_consume_mana(game::alive_object_t* thisptr, float amount)
	{

		static auto this_hook = g_hook_manager.get_hook(CONSUME_MANA_SYMBOL);

		amount = 0.f;

		return this_hook.call_original<decltype(&hk_consume_mana)>(thisptr, amount);

	}

	__int64 __fastcall hk_consume_stamina(game::alive_object_t* thisptr, float amount)
	{

		static auto this_hook = g_hook_manager.get_hook(CONSUME_STAMINA_SYMBOL);

		amount = 0.f;

		return this_hook.call_original<decltype(&hk_consume_stamina)>(thisptr, amount);

	}

	__int64 __fastcall hk_receieve_raw_damage(game::alive_object_t* thisptr, float amount)
	{

		static auto this_hook = g_hook_manager.get_hook(RECEIVE_RAW_DAMAGE_SYMBOL);

		return this_hook.call_original<decltype(&hk_receieve_raw_damage)>(thisptr, amount);

	}

}

game::game_state_t* g_game_state{};
game::world_context_t* g_world_context{};

namespace game_state
{

	void __fastcall hk_update_camera(game::game_state_t* thisptr, game::world_context_t* world_context, float a3)
	{

		static auto this_hook = g_hook_manager.get_hook(UPDATE_CAMERA_SYMBOL);

		g_game_state = thisptr;
		g_world_context = world_context;

		return this_hook.call_original<decltype(&hk_update_camera)>(thisptr, world_context, a3);

	}

}

void emplace_hooks()
{

	g_hook_manager.add_hook_from_signature(character::hk_instance_step, INSTANCE_STEP_SYMBOL, INSTANCE_STEP_SIGNATURE);
	g_hook_manager.add_hook_from_signature(character::hk_clamp_speed, CLAMP_SPEED_SYMBOL, CLAMP_SPEED_SIGNATURE);
	g_hook_manager.add_hook_from_signature(character_base::hk_move_speed, GET_MOVE_SPEED_SYMBOL, GET_MOVE_SPEED_SIGNATURE);
	g_hook_manager.add_hook_from_signature(alive_object::hk_consume_mana, CONSUME_MANA_SYMBOL, CONSUME_MANA_SIGNATURE);
	g_hook_manager.add_hook_from_signature(alive_object::hk_consume_stamina, CONSUME_STAMINA_SYMBOL, CONSUME_STAMINA_SIGNATURE);
	//g_hook_manager.add_hook_from_signature(alive_object::hk_receieve_raw_damage, RECEIVE_RAW_DAMAGE_SYMBOL, RECEIVE_RAW_DAMAGE_SIGNATURE);
	g_hook_manager.add_hook_from_signature(game_state::hk_update_camera, UPDATE_CAMERA_SYMBOL, UPDATE_CAMERA_SIGNATURE);

	g_hook_manager.create_hooks();

}