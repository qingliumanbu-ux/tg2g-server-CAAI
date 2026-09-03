/*=========================================================================
//程序名称:     f_caai_01
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//函数说明：根据计划号查出入口重量-出口重量，根据代表成分信息：TQMTQQ0,投料：hpsbwa2 ，产出tmmbw23b1,tmmbw23b2,tmmbw23b3,tmmbw23d1
自产镍钼合金钢废钢	ZF1		Ni≥0.35%同时Mo≥0.13%
自产钼合金钢废钢	ZM1		Mo≥0.15% Ni为残余含量
自产镍合金钢废钢	ZN1		Ni≥0.70% Mo为残余含量
自产高铬轴承钢废钢	ZC1	
自产工磨具钢废钢（含有Cr、Mo、V、W等元素）	ZG1	按牌号收集
自产含钛合金钢废钢	ZT1		Ti≥0.025%
自产高硫钢废钢	ZS1		S≥0.070%
自产高铜钢废钢	ZA1		Cu≥0.30%
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
//程序用头文件

BM2_FUNCTION_EXPORT
int f_caai_hs(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal in_wt,out_wt;
	CString heat_no = "";
	CString plan_no = "";
	CString table = "";
	int rownum = 0;

	CModel tcaaia8("TCAAIA8");

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_sub(conn);

	try
	{
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{

			plan_no = bcls_rec->Tables[0].Rows[i]["ROLL_PLAN_NO"].ToString();
			tcaaia8.Reset();
			tcaaia8["PLAN_NO"] = plan_no;
			tcaaia8["DEPT_CODE"] = "X";
			sqlstr = "select heat_no,pono,whole_backlog_code,equ_no,plan_end_time"
				"  from HPSBWA1"
				" where ROLL_PLAN_NO=@plan_no"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("plan_no", plan_no);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				tcaaia8["HEAT_NO"] = cmd_inq_1.GetString(1);
				tcaaia8["PONO"] = cmd_inq_1.GetString(2);
				tcaaia8["SUB_BACKLOG_CODE"] = cmd_inq_1.GetString(3);
				tcaaia8["EQU_NO"] = cmd_inq_1.GetString(4);
				tcaaia8["PROD_DATE"] = cmd_inq_1.GetString(5).SubstringNE(0,8);
			}
			cmd_inq_1.Close();

			tcaaia8.Delete("DEPT_CODE,PLAN_NO,SUB_BACKLOG_CODE,EQU_NO,PROD_DATE");		


			//根据委外计划号取入口材料重量
			sqlstr = " select sum(in_mat_wt) from HPSBWA2"
				" where ROLL_PLAN_NO=@plan_no"
				;
			in_wt = 0;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("plan_no", plan_no);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				in_wt = cmd_inq_1.GetDecimal(1);
				tcaaia8["IN_MAT_WT"] = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			if (plan_no.SubstringNE(0, 2) == "B1")
			{
				table = "tmmbw23b1";
			}
			else if (plan_no.SubstringNE(0, 2) == "B2")
			{
				table = "tmmbw23b2";
			}
			else if (plan_no.SubstringNE(0, 2) == "D1")
			{
				table = "tmmbw23d1";
			}
			else //开坯和大棒
			{
				table = "tmmbw23b3";
			}
						


			//根据计划号取出口材料重量
			sqlstr = " select sum(mat_act_wt) from " + table +
				" where plan_no=@plan_no"
				;
			out_wt = 0;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("plan_no", plan_no);
			cmd_inq_1.ExecuteReader();
			if (cmd_inq_1.Read())
			{
				out_wt = cmd_inq_1.GetDecimal(1);
				tcaaia8["OUT_MAT_WT"] = cmd_inq_1.GetDecimal(1);
			}
			cmd_inq_1.Close();

			tcaaia8["MAT_CODE"] = "Z0024";//氧化铁皮
			tcaaia8["MAT_NAME"] = "氧化铁皮";//氧化铁皮
			tcaaia8["PROD_WT"] = ((in_wt - out_wt)*0.015).Round(3);
			tcaaia8["REC_CREATOR"] = s.userid;
			tcaaia8["REC_CREATE_TIME"] = datenow;

					
			tcaaia8.Insert();

			//取废钢的物料编码，根据成分来
			sqlstr = " select sum(decode(elm_name,'Ni',elm_act,0)) Ni"
				",sum(decode(elm_name,'Mo',elm_act,0)) Mo"
				", sum(decode(elm_name, 'Ti', elm_act, 0)) Ti"
				", sum(decode(elm_name, 'S', elm_act, 0)) S"
				", sum(decode(elm_name, 'Cu', elm_act, 0)) Cu"
				" from TQMTQQ0"
				" where heat_no = @heat_no"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("heat_no", tcaaia8["HEAT_NO"].ToString());
			cmd_inq_1.ExecuteReader();	
			tcaaia8["MAT_CODE"] = " ";
			if (cmd_inq_1.Read())
			{
				if (cmd_inq_1.GetDecimal(1) >= 0.35&&cmd_inq_1.GetDecimal(2) >= 0.13)
				{
					tcaaia8["MAT_CODE"] = "Z0096";//自产镍钼合金钢废钢	ZF1		Ni≥0.35%同时Mo≥0.13%
					tcaaia8["MAT_NAME"] = "含钼含镍切头";
				}
				if (cmd_inq_1.GetDecimal(1) < 0.35&&cmd_inq_1.GetDecimal(2) >= 0.15)
				{
					tcaaia8["MAT_CODE"] = "Z0031";//自产钼合金钢废钢	ZM1		Mo≥0.15% Ni为残余含量
					tcaaia8["MAT_NAME"] = "含钼切头";
				}
				if (cmd_inq_1.GetDecimal(1) >= 0.7&&cmd_inq_1.GetDecimal(2) < 0.13)
				{
					tcaaia8["MAT_CODE"] = "Z0082";//自产镍合金钢废钢	ZN1		Ni≥0.70% Mo为残余含量
					tcaaia8["MAT_NAME"] = "含镍切头（镍含量≥0.7%）";
				}
				if (cmd_inq_1.GetDecimal(1) < 0.7&&cmd_inq_1.GetDecimal(1) > 0)
				{
					tcaaia8["MAT_CODE"] = "Z0081";
					tcaaia8["MAT_NAME"] = "含镍切头（镍含量＜0.7%）";
				}
				if (cmd_inq_1.GetDecimal(3) >= 0.025)
				{
					tcaaia8["MAT_CODE"] = "Z0097";//自产含钛合金钢废钢	ZT1		Ti≥0.025%
					tcaaia8["MAT_NAME"] = "含钛钢切头";
				}
				if (cmd_inq_1.GetDecimal(4) >= 0.07)
				{
					tcaaia8["MAT_CODE"] = "ZS1";//自产高硫钢废钢	ZS1		S≥0.070%
					tcaaia8["MAT_NAME"] = "自产高硫钢废钢";
				}
				if (cmd_inq_1.GetDecimal(5) >= 0.3)
				{
					tcaaia8["MAT_CODE"] = "ZA1";//自产高铜钢废钢	ZA1		Cu≥0.30%
					tcaaia8["MAT_NAME"] = "自产高铜钢废钢";
				}
			}
			cmd_inq_1.Close();
			//按分类来
			if (tcaaia8["MAT_CODE"].ToString().Trim() == "")
			{
				
			}
			//什么都没有，就按普通切头来
			if (tcaaia8["MAT_CODE"].ToString().Trim() == "")
			{
				tcaaia8["MAT_CODE"] = "Z0025"; 
					tcaaia8["MAT_NAME"] = "普通切头";
			}

			
				tcaaia8["PROD_WT"] = ((in_wt - out_wt)*(1 - 0.015 - 0.015)).Round(3);  //氧化铁皮1.5%，金属损失1.5%
			
			tcaaia8.Insert();

		}

	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
